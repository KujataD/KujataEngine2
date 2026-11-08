#include "VertexBuffer.h"

#include "Core/Assert.h"
#include "UploadBuffer.h"

#include <cstring>

namespace KujataEngine {

void VertexBuffer::Initialize(ID3D12Device* device, const void* vertices, UINT vertexSize, UINT vertexCount) {
	const UINT bufferSize = vertexSize * vertexCount;

	buffer_ = CreateUploadBuffer(device, bufferSize);

	void* mappedMemory = nullptr;
	const HRESULT result = buffer_->Map(0, nullptr, &mappedMemory);
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
