#include "GpuDevice.h"

#include "Core/Assert.h"

#include <cstdio>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

using Microsoft::WRL::ComPtr;

namespace KujataEngine {

// これより古い機能レベルのGPUは使わない。
static const D3D_FEATURE_LEVEL kMinFeatureLevel = D3D_FEATURE_LEVEL_12_0;

void GpuDevice::Initialize() {
	CreateFactory();
	CreateDevice();
}

ID3D12Device* GpuDevice::GetDevice() {
	return device_.Get();
}

IDXGIFactory7* GpuDevice::GetFactory() {
	return factory_.Get();
}

void GpuDevice::CreateFactory() {
	const HRESULT result = CreateDXGIFactory1(IID_PPV_ARGS(&factory_));
	KUJATA_ASSERT(SUCCEEDED(result));
}

void GpuDevice::CreateDevice() {
	ComPtr<IDXGIAdapter4> adapter;

	// 高性能なGPUから順に試して、最初に使えたものに決める。
	for (UINT i = 0; factory_->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND; i++) {
		DXGI_ADAPTER_DESC3 description = {};
		const HRESULT descriptionResult = adapter->GetDesc3(&description);
		KUJATA_ASSERT(SUCCEEDED(descriptionResult));

		// CPUで動くGPUの代わりは遅いので使わない。
		if (description.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE) {
			continue;
		}

		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), kMinFeatureLevel, IID_PPV_ARGS(&device_)))) {
			printf("GPU: %ls\n", description.Description);
			return;
		}
	}

	// 使えるGPUが1つもなかったときは、ここで止まる。
	KUJATA_ASSERT(device_ != nullptr);
}

} // namespace KujataEngine
