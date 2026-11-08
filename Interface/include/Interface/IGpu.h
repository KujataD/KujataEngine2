#pragma once

#include <dxgiformat.h>
#include <functional>

// d3d12.hをこのヘッダーに持ち込まないように、型の名前だけを伝える。
struct ID3D12Device;
struct ID3D12GraphicsCommandList;

namespace KujataEngine {

// ServiceRegistryに登録するときの名前。
const char* const kGpuServiceName = "Gpu";

/// <summary>
/// GPUを他のモジュールから使うための窓口。
/// </summary>
class IGpu {
public:
	virtual ~IGpu() = default;

	/// <summary>
	/// GPUの資源を作るためのDeviceを返す。
	/// </summary>
	virtual ID3D12Device* GetDevice() = 0;

	/// <summary>
	/// 今のフレームの命令を書くCommandListを返す。
	/// </summary>
	virtual ID3D12GraphicsCommandList* GetCommandList() = 0;

	/// <summary>
	/// 描くときの形式を返す。
	/// </summary>
	virtual DXGI_FORMAT GetRenderTargetFormat() = 0;

	/// <summary>
	/// 深度の形式を返す。
	/// </summary>
	virtual DXGI_FORMAT GetDepthFormat() = 0;

	/// <summary>
	/// 渡された処理で命令を書いてGPUに実行させ、完了まで待つので、フレームの外で呼ぶ。
	/// </summary>
	virtual void ExecuteImmediately(std::function<void(ID3D12GraphicsCommandList*)> recordCommands) = 0;
};

} // namespace KujataEngine
