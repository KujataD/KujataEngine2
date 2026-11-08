#include "ConstantBuffer.h"

#include "Core/Assert.h"
#include "UploadBuffer.h"

#include <cstring>

namespace KujataEngine {

void ConstantBuffer::Initialize(ID3D12Device* device, UINT dataSize) {
	KUJATA_ASSERT(dataSize > 0);
	dataSize_ = dataSize;

	// 定数バッファの大きさは256バイト単位にする
	const UINT bufferSize = (dataSize + D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1) & ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1);

	buffer_ = CreateUploadBuffer(device, bufferSize);

	// 毎フレーム書くので、Unmapせずに開いたままにする。
	const HRESULT result = buffer_->Map(0, nullptr, &mappedMemory_);
	KUJATA_ASSERT(SUCCEEDED(result));
}

void ConstantBuffer::Write(const void* data) {
	// GPUが読んでいる間は書き換えられないが、Gpuが毎フレーム完了を待っているので安全。
	memcpy(mappedMemory_, data, dataSize_);
}

void ConstantBuffer::Bind(ID3D12GraphicsCommandList* list, UINT rootParameterIndex) {
	list->SetGraphicsRootConstantBufferView(rootParameterIndex, buffer_->GetGPUVirtualAddress());
}

} // namespace KujataEngine
