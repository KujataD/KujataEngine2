#pragma once

#include <d3d12.h>
#include <wrl/client.h>

namespace KujataEngine {

/// <summary>
/// CPUから直接書き込めるGPUのバッファを作る。
/// </summary>
Microsoft::WRL::ComPtr<ID3D12Resource> CreateUploadBuffer(ID3D12Device* device, UINT sizeInBytes);

} // namespace KujataEngine
