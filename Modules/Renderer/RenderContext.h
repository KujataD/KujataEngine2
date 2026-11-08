#pragma once

#include "RenderQueue.h"

#include <d3d12.h>

namespace KujataEngine {

/// <summary>
/// 1回の描画で各パスが共通で使う情報。
/// </summary>
struct RenderContext {
	ID3D12GraphicsCommandList* commandList;
	const RenderQueue* renderQueue;
	float targetWidth;
	float targetHeight;
};

} // namespace KujataEngine
