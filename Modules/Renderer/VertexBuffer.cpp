#include "VertexBuffer.h"

#include "Core/Assert.h"

#include <cstring>

namespace KujataEngine {

void VertexBuffer::Initialize(ID3D12Device* device, const void* vertices, UINT vertexSize, UINT vertexCount) {
	const UINT bufferSize = vertexSize * vertexCount;

	// Uploadヒープは少し遅いが、CPUから直接書き込める。
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

	void* mappedMemory = nullptr;
	result = buffer_->Map(0, nullptr, &mappedMemory);
	KUJATA_ASSERT(SUCCEEDED(result));
	memcpy(mappedMemory, vertices, bufferSize);
	buffer_->Unmap(0, nullptr);

	view_.BufferLocation = buffer_->GetGPUVirtualAddress();
	view_.SizeInBytes = bufferSize;
	view_.StrideInBytes = vertexSize;
}

void VertexBuffer::Bind(ID3D12GraphicsCommandList* list) {
	list->IASetVertexBuffers(0, 1, &view_);
}

} // namespace KujataEngine
