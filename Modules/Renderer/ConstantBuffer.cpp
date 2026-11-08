#include "ConstantBuffer.h"

#include "Core/Assert.h"
#include "UploadBuffer.h"

#include <cstring>

namespace KujataEngine {

void ConstantBuffer::Initialize(ID3D12Device* device, UINT dataSize, UINT capacity) {
	KUJATA_ASSERT(dataSize > 0);
	KUJATA_ASSERT(capacity > 0);
	dataSize_ = dataSize;
	capacity_ = capacity;

	// 定数バッファの大きさは256バイト単位にする
	alignedSize_ = (dataSize + D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1) & ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1);

	buffer_ = CreateUploadBuffer(device, alignedSize_ * capacity);

	// 毎フレーム書くので、Unmapせずに開いたままにする。
	const HRESULT result = buffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedMemory_));
	KUJATA_ASSERT(SUCCEEDED(result));
}

void ConstantBuffer::Write(UINT index, const void* data) {
	KUJATA_ASSERT(index < capacity_);

	// GPUが読んでいる間は書き換えられないが、Gpuが毎フレーム完了を待っているので安全。
	memcpy(mappedMemory_ + static_cast<SIZE_T>(index) * alignedSize_, data, dataSize_);
}

void ConstantBuffer::Bind(ID3D12GraphicsCommandList* list, UINT rootParameterIndex, UINT index) {
	KUJATA_ASSERT(index < capacity_);

	list->SetGraphicsRootConstantBufferView(rootParameterIndex, buffer_->GetGPUVirtualAddress() + static_cast<UINT64>(index) * alignedSize_);
}

} // namespace KujataEngine
