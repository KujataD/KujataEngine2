#pragma once

#include <d3d12.h>
#include <wrl/client.h>

namespace KujataEngine {

/// <summary>
/// DirectXのデバッグレイヤーを有効にして、見つかった問題を受け取るクラス。
/// </summary>
class GpuDebugMessages {
public:
	/// <summary>
	/// デバッグレイヤーを有効にし、Deviceを作る前に呼ぶ必要がある。
	/// </summary>
	void EnableDebugLayer();

	/// <summary>
	/// 作ったDeviceからメッセージを受け取れるようにする。
	/// </summary>
	void AttachToDevice(ID3D12Device* device);

	/// <summary>
	/// 溜まったメッセージのうち問題があるものをコンソールに出し、エラーがあればtrueを返す。
	/// </summary>
	bool PrintProblems();

private:
	// デバッグレイヤーが有効なときだけ値が入る。
	Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue_;
};

} // namespace KujataEngine
