#include "DepthBuffer.h"

#include "Core/Assert.h"

namespace KujataEngine {

// 深度の形式で、深度24ビットとステンシル8ビット。
static const DXGI_FORMAT kDepthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

// いちばん遠い深度の値。
static const float kFarthestDepth = 1.0f;

void DepthBuffer::Initialize(ID3D12Device* device, UINT width, UINT height) {
	D3D12_RESOURCE_DESC description = {};
	description.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	description.Width = width;
	description.Height = height;
	description.DepthOrArraySize = 1;
	description.MipLevels = 1;
	description.Format = kDepthFormat;
	description.SampleDesc.Count = 1;
	description.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = kDepthFormat;
	clearValue.DepthStencil.Depth = kFarthestDepth;

	HRESULT result = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &description, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(&buffer_));
	KUJATA_ASSERT(SUCCEEDED(result));

	D3D12_DESCRIPTOR_HEAP_DESC heapDescription = {};
	heapDescription.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	heapDescription.NumDescriptors = 1;
	result = device->CreateDescriptorHeap(&heapDescription, IID_PPV_ARGS(&viewHeap_));
	KUJATA_ASSERT(SUCCEEDED(result));

	D3D12_DEPTH_STENCIL_VIEW_DESC viewDescription = {};
	viewDescription.Format = kDepthFormat;
	viewDescription.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	device->CreateDepthStencilView(buffer_.Get(), &viewDescription, GetView());
}

D3D12_CPU_DESCRIPTOR_HANDLE DepthBuffer::GetView() {
	return viewHeap_->GetCPUDescriptorHandleForHeapStart();
}

DXGI_FORMAT DepthBuffer::GetFormat() {
	return kDepthFormat;
}

void DepthBuffer::Clear(ID3D12GraphicsCommandList* list) {
	list->ClearDepthStencilView(GetView(), D3D12_CLEAR_FLAG_DEPTH, kFarthestDepth, 0, 0, nullptr);
}

} // namespace KujataEngine
