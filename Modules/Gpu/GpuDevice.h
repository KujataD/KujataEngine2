#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

namespace KujataEngine {

/// <summary>
/// 使うGPUを選んで、DirectXの窓口であるDeviceを作るクラス。
/// </summary>
class GpuDevice {
public:
	/// <summary>
	/// FactoryとDeviceを作る。
	/// </summary>
	void Initialize();

	ID3D12Device* GetDevice();
	IDXGIFactory7* GetFactory();

private:
	void CreateFactory();
	void CreateDevice();

	Microsoft::WRL::ComPtr<IDXGIFactory7> factory_;
	Microsoft::WRL::ComPtr<ID3D12Device> device_;
};

} // namespace KujataEngine
