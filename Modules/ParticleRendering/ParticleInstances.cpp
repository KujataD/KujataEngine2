#include "ParticleInstances.h"

#include "Core/Assert.h"
#include "GraphicsKit/GpuBuffer.h"

#include <algorithm>
#include <cstdio>

namespace KujataEngine {

void ParticleInstances::Initialize(ID3D12Device* device, ShaderResourceHeap* shaderResourceHeap) {
	device_ = device;
	shaderResourceHeap_ = shaderResourceHeap;
}

ParticleInstance& ParticleInstances::FindOrCreate(uint64_t instanceId, UINT pointCount) {
	std::map<uint64_t, ParticleInstance>::iterator found = instances_.find(instanceId);
	if (found != instances_.end()) {
		found->second.isUsedThisFrame = true;
		return found->second;
	}

	if (pointCount > kMaxPointCountPerParticle && !hasWarnedTooManyPoints_) {
		printf("1つの粒子の粒が%u個を超えたので、超えた分は描きません。\n", kMaxPointCountPerParticle);
		hasWarnedTooManyPoints_ = true;
	}

	ParticleInstance& instance = instances_[instanceId];
	instance.pointCount = (std::min)(pointCount, kMaxPointCountPerParticle);
	instance.isUsedThisFrame = true;

	// コンピュートシェーダーから書き込めるようにUAVを許す。
	instance.stateBuffer = CreateGpuBuffer(device_, sizeof(ParticleState) * static_cast<UINT64>(instance.pointCount), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	// 頂点シェーダーのStructuredBuffer<ParticleState>として読むための名札を、棚に置く。
	D3D12_SHADER_RESOURCE_VIEW_DESC readDescription = {};
	readDescription.Format = DXGI_FORMAT_UNKNOWN;
	readDescription.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	readDescription.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	readDescription.Buffer.FirstElement = 0;
	readDescription.Buffer.NumElements = instance.pointCount;
	readDescription.Buffer.StructureByteStride = sizeof(ParticleState);
	instance.readHeapIndex = AllocateHeapIndex();
	device_->CreateShaderResourceView(instance.stateBuffer.Get(), &readDescription, shaderResourceHeap_->GetCpuHandle(instance.readHeapIndex));

	// コンピュートシェーダーのRWStructuredBuffer<ParticleState>として書くための名札も、棚に置く。
	D3D12_UNORDERED_ACCESS_VIEW_DESC writeDescription = {};
	writeDescription.Format = DXGI_FORMAT_UNKNOWN;
	writeDescription.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	writeDescription.Buffer.FirstElement = 0;
	writeDescription.Buffer.NumElements = instance.pointCount;
	writeDescription.Buffer.StructureByteStride = sizeof(ParticleState);
	instance.writeHeapIndex = AllocateHeapIndex();
	device_->CreateUnorderedAccessView(instance.stateBuffer.Get(), nullptr, &writeDescription, shaderResourceHeap_->GetCpuHandle(instance.writeHeapIndex));

	return instance;
}

const ParticleInstance* ParticleInstances::Find(uint64_t instanceId) const {
	std::map<uint64_t, ParticleInstance>::const_iterator found = instances_.find(instanceId);
	if (found == instances_.end()) {
		return nullptr;
	}
	return &found->second;
}

void ParticleInstances::RemoveUnused() {
	// Gpuが毎フレーム完了を待つので、このフレームで使わなかったバッファはもうGPUも使っていない。
	for (std::map<uint64_t, ParticleInstance>::iterator it = instances_.begin(); it != instances_.end();) {
		ParticleInstance& instance = it->second;
		if (instance.isUsedThisFrame) {
			instance.isUsedThisFrame = false;
			++it;
			continue;
		}

		reusableHeapIndices_.push_back(instance.readHeapIndex);
		reusableHeapIndices_.push_back(instance.writeHeapIndex);
		it = instances_.erase(it);
	}
}

UINT ParticleInstances::AllocateHeapIndex() {
	if (reusableHeapIndices_.empty()) {
		return shaderResourceHeap_->Allocate();
	}

	const UINT heapIndex = reusableHeapIndices_.back();
	reusableHeapIndices_.pop_back();
	return heapIndex;
}

} // namespace KujataEngine
