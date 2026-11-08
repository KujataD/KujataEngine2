#pragma once

#include "Core/IModule.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

namespace KujataEngine {

/// <summary>
/// GPUを見つけて、命令を受け付ける準備をするモジュール。
/// </summary>
class Gpu : public IModule {
public:
	void Startup(ServiceRegistry& services, FrameLoop& frameLoop) override;
	void Shutdown() override;

private:
	void EnableDebugLayer();
	bool CreateFactory();
	bool CreateDevice();
	bool CreateCommandObjects();
	bool CreateFence();
	void SetUpDebugMessages();
	void PrintDebugMessages();

	Microsoft::WRL::ComPtr<IDXGIFactory7> factory_;
	Microsoft::WRL::ComPtr<ID3D12Device> device_;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;
	Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
	HANDLE fenceEvent_ = nullptr;

	// デバッグレイヤーが有効なときだけ値が入る。
	Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue_;
};

} // namespace KujataEngine
