#include "SwapChain.h"

#include "Core/Assert.h"

using Microsoft::WRL::ComPtr;

namespace KujataEngine {

// 画面に出す画像の形式で、RGBAが各8ビット。
static const DXGI_FORMAT kBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

// 描くときの形式で、シェーダーの出力をsRGBに変換して書き込む。
static const DXGI_FORMAT kRenderTargetFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

void SwapChain::Initialize(IDXGIFactory7* factory, ID3D12Device* device, ID3D12CommandQueue* queue, IWindow* window) {
	CreateSwapChain(factory, queue, window);
	CreateRenderTargetViews(device);
}

void SwapChain::TransitionToRenderTarget(ID3D12GraphicsCommandList* list) {
	Transition(list, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

void SwapChain::TransitionToPresent(ID3D12GraphicsCommandList* list) {
	Transition(list, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
}

D3D12_CPU_DESCRIPTOR_HANDLE SwapChain::GetCurrentRenderTargetView() {
	D3D12_CPU_DESCRIPTOR_HANDLE handle = renderTargetViewHeap_->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += static_cast<SIZE_T>(backBufferIndex_) * renderTargetViewSize_;
	return handle;
}

DXGI_FORMAT SwapChain::GetRenderTargetFormat() {
	return kRenderTargetFormat;
}

void SwapChain::Present() {
	// 1を渡すと、画面の更新に合わせて待つ。
	const HRESULT result = swapChain_->Present(1, 0);
	KUJATA_ASSERT(SUCCEEDED(result));

	backBufferIndex_ = swapChain_->GetCurrentBackBufferIndex();
}

void SwapChain::CreateSwapChain(IDXGIFactory7* factory, ID3D12CommandQueue* queue, IWindow* window) {
	DXGI_SWAP_CHAIN_DESC1 description = {};
	description.Width = window->GetClientWidth();
	description.Height = window->GetClientHeight();
	description.Format = kBackBufferFormat;
	description.SampleDesc.Count = 1;
	description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	description.BufferCount = kBackBufferCount;
	description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	// Presentは命令の列に並ぶので、DeviceではなくCommandQueueを渡す。
	HWND hwnd = static_cast<HWND>(window->GetNativeHandle());
	ComPtr<IDXGISwapChain1> createdSwapChain;
	HRESULT result = factory->CreateSwapChainForHwnd(queue, hwnd, &description, nullptr, nullptr, &createdSwapChain);
	KUJATA_ASSERT(SUCCEEDED(result));

	// GetCurrentBackBufferIndexを使うには新しい型が要る。
	result = createdSwapChain.As(&swapChain_);
	KUJATA_ASSERT(SUCCEEDED(result));

	backBufferIndex_ = swapChain_->GetCurrentBackBufferIndex();
}

void SwapChain::CreateRenderTargetViews(ID3D12Device* device) {
	D3D12_DESCRIPTOR_HEAP_DESC heapDescription = {};
	heapDescription.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDescription.NumDescriptors = kBackBufferCount;
	HRESULT result = device->CreateDescriptorHeap(&heapDescription, IID_PPV_ARGS(&renderTargetViewHeap_));
	KUJATA_ASSERT(SUCCEEDED(result));

	// RTV1個の大きさはGPUによって違うので、聞いてからずらす。
	renderTargetViewSize_ = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	D3D12_RENDER_TARGET_VIEW_DESC viewDescription = {};
	viewDescription.Format = kRenderTargetFormat;
	viewDescription.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	D3D12_CPU_DESCRIPTOR_HANDLE handle = renderTargetViewHeap_->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < kBackBufferCount; i++) {
		result = swapChain_->GetBuffer(i, IID_PPV_ARGS(&backBuffers_[i]));
		KUJATA_ASSERT(SUCCEEDED(result));

		device->CreateRenderTargetView(backBuffers_[i].Get(), &viewDescription, handle);
		handle.ptr += renderTargetViewSize_;
	}
}

void SwapChain::Transition(ID3D12GraphicsCommandList* list, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after) {
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = backBuffers_[backBufferIndex_].Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = before;
	barrier.Transition.StateAfter = after;
	list->ResourceBarrier(1, &barrier);
}

} // namespace KujataEngine
