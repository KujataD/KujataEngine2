#include "ConstantBuffer.h"

#include "Core/Assert.h"

#include <cstring>

namespace KujataEngine {

void ConstantBuffer::Initialize(ID3D12Device* device, UINT dataSize) {
	KUJATA_ASSERT(dataSize > 0);
	dataSize_ = dataSize;

	// 定数バッファの大きさは256バイト単位にする
	const UINT bufferSize = (dataSize + D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1) & ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1);

	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC description = {};
	description.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	description.Width = bufferSize;
	description.Height = 1;
	description.DepthOrArraySize = 1;
	description.MipLevels = 1;
	description.SampleDesc.Count = 1;
	description.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	HRESULT result = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &description, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buffer_));
	KUJATA_ASSERT(SUCCEEDED(result));

	// 毎フレーム書くので、Unmapせずに開いたままにする。
	result = buffer_->Map(0, nullptr, &mappedMemory_);
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
