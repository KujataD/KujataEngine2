#include "GpuAssetTable.h"

#include "Core/Assert.h"
#include "GraphicsKit/GpuBuffer.h"
#include "GraphicsKit/UploadBuffer.h"

#include <cstdio>
#include <cstring>

using Microsoft::WRL::ComPtr;

namespace KujataEngine {

static_assert(sizeof(PointCloudTarget) == 36, "PointCloudTargetの大きさが変わったらPointCloud.VS.hlslとPointCloudUpdate.CS.hlslも直す。");

void GpuAssetTable::UploadAll(IGpu* gpu, ShaderResourceHeap* heap, IAssetLibrary* assets) {
	ID3D12Device* device = gpu->GetDevice();

	CreateMeshes(device, assets);
	CreateTextures(device, heap, assets);

	// 全画像のコピーを1回の命令書にまとめて、完了まで待つ。
	gpu->ExecuteImmediately([&](ID3D12GraphicsCommandList* list) {
		for (uint32_t i = 0; i < textures_.size(); i++) {
			TextureHandle handle;
			handle.id = i;
			textures_[i].RecordUpload(list, assets->GetImageData(handle));
		}
	});

	for (Texture& texture : textures_) {
		texture.ReleaseUploadBuffer();
	}

	CreatePointClouds(gpu, heap, assets);
}

GpuMesh& GpuAssetTable::GetMesh(MeshHandle handle) {
	if (handle.id >= meshes_.size()) {
		WarnNotUploaded();
		return meshes_[0];
	}
	return meshes_[handle.id];
}

Texture& GpuAssetTable::GetTexture(TextureHandle handle) {
	if (handle.id >= textures_.size()) {
		WarnNotUploaded();
		return textures_[0];
	}
	return textures_[handle.id];
}

GpuPointCloud& GpuAssetTable::GetPointCloud(PointCloudHandle handle) {
	if (handle.id >= pointClouds_.size()) {
		WarnNotUploaded();
		return pointClouds_[0];
	}
	return pointClouds_[handle.id];
}

void GpuAssetTable::CreateMeshes(ID3D12Device* device, IAssetLibrary* assets) {
	meshes_.resize(assets->GetMeshCount());

	for (uint32_t i = 0; i < meshes_.size(); i++) {
		MeshHandle handle;
		handle.id = i;
		const MeshData& mesh = assets->GetMeshData(handle);

		meshes_[i].vertexBuffer.Initialize(device, mesh.vertices.data(), sizeof(MeshVertex), static_cast<UINT>(mesh.vertices.size()));
		meshes_[i].indexBuffer.Initialize(device, mesh.indices.data(), static_cast<UINT>(mesh.indices.size()));
	}
}

void GpuAssetTable::CreateTextures(ID3D12Device* device, ShaderResourceHeap* heap, IAssetLibrary* assets) {
	for (uint32_t i = 0; i < assets->GetTextureCount(); i++) {
		if (!heap->HasFreeSlot()) {
			printf("テクスチャの棚が満杯なので、残りは代わりの物で描きます。\n");
			break;
		}

		TextureHandle handle;
		handle.id = i;

		textures_.emplace_back();
		textures_.back().Initialize(device, heap, assets->GetImageData(handle));
	}
}

void GpuAssetTable::CreatePointClouds(IGpu* gpu, ShaderResourceHeap* heap, IAssetLibrary* assets) {
	ID3D12Device* device = gpu->GetDevice();
	pointClouds_.resize(assets->GetPointCloudCount());

	// CPUから書ける一時バッファに入れてから、GPUの中でコピーする。
	std::vector<ComPtr<ID3D12Resource>> uploadBuffers(pointClouds_.size());
	for (uint32_t i = 0; i < pointClouds_.size(); i++) {
		PointCloudHandle handle;
		handle.id = i;
		const std::vector<PointCloudTarget>& targets = assets->GetPointCloudData(handle).targets;
		const UINT targetCount = static_cast<UINT>(targets.size());
		const UINT bufferSize = sizeof(PointCloudTarget) * targetCount;

		uploadBuffers[i] = CreateUploadBuffer(device, bufferSize);
		void* mappedMemory = nullptr;
		const HRESULT result = uploadBuffers[i]->Map(0, nullptr, &mappedMemory);
		KUJATA_ASSERT(SUCCEEDED(result));
		memcpy(mappedMemory, targets.data(), bufferSize);
		uploadBuffers[i]->Unmap(0, nullptr);

		pointClouds_[i].targetBuffer = CreateGpuBuffer(device, bufferSize, D3D12_RESOURCE_FLAG_NONE);
		pointClouds_[i].targetCount = targetCount;

		// シェーダーのStructuredBuffer<PointCloudTarget>として読むための名札を、棚に置く。
		D3D12_SHADER_RESOURCE_VIEW_DESC viewDescription = {};
		viewDescription.Format = DXGI_FORMAT_UNKNOWN;
		viewDescription.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		viewDescription.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		viewDescription.Buffer.FirstElement = 0;
		viewDescription.Buffer.NumElements = targetCount;
		viewDescription.Buffer.StructureByteStride = sizeof(PointCloudTarget);

		const UINT heapIndex = heap->Allocate();
		device->CreateShaderResourceView(pointClouds_[i].targetBuffer.Get(), &viewDescription, heap->GetCpuHandle(heapIndex));
		pointClouds_[i].targetsGpuHandle = heap->GetGpuHandle(heapIndex);
	}

	// 完了まで待つので、関数を抜けて一時バッファが消えても問題ない。
	gpu->ExecuteImmediately([&](ID3D12GraphicsCommandList* list) {
		for (uint32_t i = 0; i < pointClouds_.size(); i++) {
			const UINT64 bufferSize = sizeof(PointCloudTarget) * static_cast<UINT64>(pointClouds_[i].targetCount);
			list->CopyBufferRegion(pointClouds_[i].targetBuffer.Get(), 0, uploadBuffers[i].Get(), 0, bufferSize);
		}
	});
}

void GpuAssetTable::WarnNotUploaded() {
	if (hasWarnedNotUploaded_) {
		return;
	}

	printf("転送していないアセットがあるので、代わりの物で描きます。\n");
	hasWarnedNotUploaded_ = true;
}

} // namespace KujataEngine
