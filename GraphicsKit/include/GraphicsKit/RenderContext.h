#pragma once

#include "Core/Math/Matrix4x4.h"

#include <d3d12.h>

namespace KujataEngine {

/// <summary>
/// 1回の描画で、どの工程にも共通に渡す情報。
/// </summary>
struct RenderContext {
	ID3D12GraphicsCommandList* commandList = nullptr;
	float targetWidth = 0.0f;
	float targetHeight = 0.0f;

	// 前のフレームからの秒数で、時間の速さが掛かっている。
	float deltaTime = 0.0f;

	// このフレームのカメラの行列で、viewProjectionはview * projectionの結果。
	Matrix4x4 view;
	Matrix4x4 projection;
	Matrix4x4 viewProjection;
};

} // namespace KujataEngine