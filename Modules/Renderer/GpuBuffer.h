#pragma once

#include <d3d12.h>
#include <wrl/client.h>

namespace KujataEngine {

/// <summary>
/// GPUだけが読み書きするバッファを、状態がCOMMONの中身が決まっていない状態で作る。
/// </summary>
Microsoft::WRL::ComPtr<ID3D12Resource> CreateGpuBuffer(ID3D12Device* device, UINT64 sizeInBytes, D3D12_RESOURCE_FLAGS flags);

} // namespace KujataEngine
