#pragma once

#include "Interface/IWindow.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

namespace KujataEngine {

/// <summary>
/// 画面に出す画像を持ち、描き先の切り替えと表示をするクラス。
/// </summary>
class SwapChain {
public:
	/// <summary>
	/// SwapChainと描き先の名札を作る。
	/// </summary>
	void Initialize(IDXGIFactory7* factory, ID3D12Device* device, ID3D12CommandQueue* queue, IWindow* window);

	/// <summary>
	/// 今の画像を描ける状態にする命令を書く。
	/// </summary>
	void TransitionToRenderTarget(ID3D12GraphicsCommandList* list);

	/// <summary>
	/// 今の画像を画面に出せる状態にする命令を書く。
	/// </summary>
	void TransitionToPresent(ID3D12GraphicsCommandList* list);

	/// <summary>
	/// 今の画像に描くための名札を返す。
	/// </summary>
	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRenderTargetView();

	/// <summary>
	/// 描くときの形式を返す。
	/// </summary>
	DXGI_FORMAT GetRenderTargetFormat();

	/// <summary>
	/// 画面に出して次の画像へ切り替える。
	/// </summary>
	void Present();

private:
	void CreateSwapChain(IDXGIFactory7* factory, ID3D12CommandQueue* queue, IWindow* window);
	void CreateRenderTargetViews(ID3D12Device* device);
	void Transition(ID3D12GraphicsCommandList* list, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);

	// 表と裏の2枚を入れ替えて画面に出す。
	static const UINT kBackBufferCount = 2;

	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> renderTargetViewHeap_;
	Microsoft::WRL::ComPtr<ID3D12Resource> backBuffers_[kBackBufferCount];
	UINT renderTargetViewSize_ = 0;
	UINT backBufferIndex_ = 0;
};

} // namespace KujataEngine
