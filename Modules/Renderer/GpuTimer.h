#pragma once

#include "Interface/IGpu.h"

#include <cstdint>
#include <d3d12.h>
#include <wrl/client.h>

namespace KujataEngine {

/// <summary>
/// 命令書の途中に印を付け、印と印の間にGPUがかけた時間を次のフレームで読むクラス。
/// </summary>
class GpuTimer {
public:
	/// <summary>
	/// 指定の数だけ印を付けられるように、タイムスタンプの置き場と読み戻し用のバッファを作る。
	/// </summary>
	void Initialize(IGpu* gpu, UINT markCount);

	/// <summary>
	/// GPUがここまで進んだ時刻を、指定の番号の印として記録する命令を書く。
	/// </summary>
	void Mark(ID3D12GraphicsCommandList* list, UINT markIndex);

	/// <summary>
	/// このフレームの印を、CPUから読めるバッファへ写す命令を書く。
	/// </summary>
	void Resolve(ID3D12GraphicsCommandList* list);

	/// <summary>
	/// 前のフレームの印が読める状態ならtrueを返す。
	/// </summary>
	bool HasResult() const { return hasResult_; }

	/// <summary>
	/// 前のフレームの2つの印の間にかかった時間をミリ秒で返す。
	/// </summary>
	double ReadMilliseconds(UINT fromMarkIndex, UINT toMarkIndex);

private:
	Microsoft::WRL::ComPtr<ID3D12QueryHeap> queryHeap_;
	Microsoft::WRL::ComPtr<ID3D12Resource> readbackBuffer_;
	UINT markCount_ = 0;
	uint64_t timestampFrequency_ = 0;
	bool hasResult_ = false;
};

} // namespace KujataEngine
