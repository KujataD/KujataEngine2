#include "IndexBuffer.h"

#include "Core/Assert.h"
#include "UploadBuffer.h"

#include <cstring>

namespace KujataEngine {

void IndexBuffer::Initialize(ID3D12Device* device, const uint32_t* indices, UINT indexCount) {
	indexCount_ = indexCount;
	const UINT bufferSize = sizeof(uint32_t) * indexCount;

	buffer_ = CreateUploadBuffer(device, bufferSize);

	void* mappedMemory = nullptr;
	const HRESULT result = buffer_->Map(0, nullptr, &mappedMemory);
	KUJATA_ASSERT(SUCCEEDED(result));
	memcpy(mappedMemory, indices, bufferSize);
	buffer_->Unmap(0, nullptr);

	view_.BufferLocation = buffer_->GetGPUVirtualAddress();
	view_.SizeInBytes = bufferSize;
	view_.Format = DXGI_FORMAT_R32_UINT;
}

void IndexBuffer::Bind(ID3D12GraphicsCommandList* list) {
	list->IASetIndexBuffer(&view_);
}

UINT IndexBuffer::GetIndexCount() {
	return indexCount_;
}

} // namespace KujataEngine
