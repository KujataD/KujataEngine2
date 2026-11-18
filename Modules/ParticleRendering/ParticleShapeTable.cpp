#include "ParticleShapeTable.h"

#include "Core/Assert.h"
#include "GraphicsKit/GpuBuffer.h"
#include "GraphicsKit/UploadBuffer.h"

#include <cstdio>
#include <cstring>

namespace KujataEngine {

static_assert(sizeof(ParticleTarget) == 36, "ParticleTargetの大きさが変わったらParticleDraw.VS.hlslとParticleSimulation.CS.hlslも直す。");

void ParticleShapeTable::Initialize(ID3D12Device* device, ShaderResourceHeap* shaderResourceHeap) {
	device_ = device;
	shaderResourceHeap_ = shaderResourceHeap;

	// 番号が見つからないときや形を作れなかったときにも何か描けるように、原点に白い1粒を置く。
	ParticleTarget fallbackTarget = {};
	fallbackTarget.normal = { 0.0f, 1.0f, 0.0f };
	fallbackTarget.color = { 1.0f, 1.0f, 1.0f };

	ParticleShape fallbackShape;
	fallbackShape.targets.push_back(fallbackTarget);
	Add(fallbackShape);
}

ParticleShapeHandle ParticleShapeTable::Add(const ParticleShape& shape) {
	KUJATA_ASSERT(device_ != nullptr);

	if (shape.targets.empty()) {
		printf("集まる先のない形なので、代わりの1粒を使います。\n");
		return ParticleShapeHandle();
	}

	if (!shaderResourceHeap_->HasFreeSlot()) {
		if (!hasWarnedHeapFull_) {
			printf("棚が満杯なので、これより後の形は代わりの1粒を使います。\n");
			hasWarnedHeapFull_ = true;
		}
		return ParticleShapeHandle();
	}

	const UINT targetCount = static_cast<UINT>(shape.targets.size());
	const UINT bufferSize = sizeof(ParticleTarget) * targetCount;

	// CPUから書ける一時バッファに入れておき、次の描画の命令書でGPUの中のバッファへコピーする。
	PendingUpload upload;
	upload.uploadBuffer = CreateUploadBuffer(device_, bufferSize);
	void* mappedMemory = nullptr;
	const HRESULT result = upload.uploadBuffer->Map(0, nullptr, &mappedMemory);
	KUJATA_ASSERT(SUCCEEDED(result));
	memcpy(mappedMemory, shape.targets.data(), bufferSize);
	upload.uploadBuffer->Unmap(0, nullptr);

	GpuParticleShape gpuShape;
	gpuShape.targetBuffer = CreateGpuBuffer(device_, bufferSize, D3D12_RESOURCE_FLAG_NONE);
	gpuShape.targetCount = targetCount;

	// シェーダーのStructuredBuffer<ParticleTarget>として読むための名札を、棚に置く。
	D3D12_SHADER_RESOURCE_VIEW_DESC viewDescription = {};
	viewDescription.Format = DXGI_FORMAT_UNKNOWN;
	viewDescription.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	viewDescription.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	viewDescription.Buffer.FirstElement = 0;
	viewDescription.Buffer.NumElements = targetCount;
	viewDescription.Buffer.StructureByteStride = sizeof(ParticleTarget);

	const UINT heapIndex = shaderResourceHeap_->Allocate();
	device_->CreateShaderResourceView(gpuShape.targetBuffer.Get(), &viewDescription, shaderResourceHeap_->GetCpuHandle(heapIndex));
	gpuShape.targetsGpuHandle = shaderResourceHeap_->GetGpuHandle(heapIndex);

	ParticleShapeHandle handle;
	handle.id = static_cast<uint32_t>(shapes_.size());
	shapes_.push_back(gpuShape);

	upload.shapeId = handle.id;
	pendingUploads_.push_back(upload);
	return handle;
}

void ParticleShapeTable::RecordPendingUploads(ID3D12GraphicsCommandList* list) {
	// Gpuが毎フレーム完了を待つので、前のフレームで書いた転送はもう終わっている。
	recordedUploadBuffers_.clear();

	if (pendingUploads_.empty()) {
		return;
	}

	// 状態の切り替えは転送の前後でまとめて書き、待ち合わせの回数を減らす。
	std::vector<D3D12_RESOURCE_BARRIER> barriers(pendingUploads_.size());
	for (size_t i = 0; i < pendingUploads_.size(); i++) {
		barriers[i].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barriers[i].Transition.pResource = shapes_[pendingUploads_[i].shapeId].targetBuffer.Get();
		barriers[i].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barriers[i].Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
		barriers[i].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
	}
	list->ResourceBarrier(static_cast<UINT>(barriers.size()), barriers.data());

	for (const PendingUpload& upload : pendingUploads_) {
		const GpuParticleShape& shape = shapes_[upload.shapeId];
		const UINT64 bufferSize = sizeof(ParticleTarget) * static_cast<UINT64>(shape.targetCount);
		list->CopyBufferRegion(shape.targetBuffer.Get(), 0, upload.uploadBuffer.Get(), 0, bufferSize);
	}

	// このフレームのうちに、コンピュートシェーダーと頂点シェーダーが読めるようにする。
	for (D3D12_RESOURCE_BARRIER& barrier : barriers) {
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	}
	list->ResourceBarrier(static_cast<UINT>(barriers.size()), barriers.data());

	for (PendingUpload& upload : pendingUploads_) {
		recordedUploadBuffers_.push_back(upload.uploadBuffer);
	}
	pendingUploads_.clear();
}

const GpuParticleShape& ParticleShapeTable::Get(ParticleShapeHandle handle) const {
	if (handle.id >= shapes_.size()) {
		if (!hasWarnedUnknownShape_) {
			printf("登録されていない形の番号なので、代わりの1粒を使います。\n");
			hasWarnedUnknownShape_ = true;
		}
		return shapes_[0];
	}
	return shapes_[handle.id];
}

} // namespace KujataEngine
