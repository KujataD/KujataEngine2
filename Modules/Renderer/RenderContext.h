#pragma once

#include "GpuAssetTable.h"
#include "RenderQueue.h"

#include <d3d12.h>

namespace KujataEngine {

/// <summary>
/// 1回の描画で各パスが共通で使う情報。
/// </summary>
struct RenderContext {
	ID3D12GraphicsCommandList* commandList;
	const RenderQueue* renderQueue;
	GpuAssetTable* gpuAssets;
	float targetWidth;
	float targetHeight;

	// 前のフレームからの秒数で、時間の速さが掛かっている。
	float deltaTime;
};

} // namespace KujataEngine
