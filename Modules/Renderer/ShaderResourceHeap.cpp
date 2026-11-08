#include "ShaderResourceHeap.h"

#include "Core/Assert.h"

namespace KujataEngine {

void ShaderResourceHeap::Initialize(ID3D12Device* device, UINT capacity) {
	capacity_ = capacity;

	D3D12_DESCRIPTOR_HEAP_DESC description = {};
	description.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	description.NumDescriptors = capacity;
	description.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	const HRESULT result = device->CreateDescriptorHeap(&description, IID_PPV_ARGS(&heap_));
	KUJATA_ASSERT(SUCCEEDED(result));

	descriptorSize_ = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

UINT ShaderResourceHeap::Allocate() {
	KUJATA_ASSERT(allocatedCount_ < capacity_);

	const UINT index = allocatedCount_;
	allocatedCount_++;
	return index;
}

D3D12_CPU_DESCRIPTOR_HANDLE ShaderResourceHeap::GetCpuHandle(UINT index) {
	D3D12_CPU_DESCRIPTOR_HANDLE handle = heap_->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += static_cast<SIZE_T>(index) * descriptorSize_;
	return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE ShaderResourceHeap::GetGpuHandle(UINT index) {
	D3D12_GPU_DESCRIPTOR_HANDLE handle = heap_->GetGPUDescriptorHandleForHeapStart();
	handle.ptr += static_cast<UINT64>(index) * descriptorSize_;
	return handle;
}

void ShaderResourceHeap::Bind(ID3D12GraphicsCommandList* list) {
	ID3D12DescriptorHeap* heaps[] = { heap_.Get() };
	list->SetDescriptorHeaps(1, heaps);
}

} // namespace KujataEngine
