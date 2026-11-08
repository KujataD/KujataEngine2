#pragma once

#include "Core/Matrix4x4.h"
#include "Core/Vector3.h"

namespace KujataEngine {

/// <summary>
/// GameObjectの位置と回転と大きさで、回転の単位はラジアン。
/// </summary>
struct Transform {
	Vector3 position = { 0.0f, 0.0f, 0.0f };
	Vector3 rotation = { 0.0f, 0.0f, 0.0f };
	Vector3 scale = { 1.0f, 1.0f, 1.0f };

	/// <summary>
	/// 拡大縮小、回転、移動の順に掛けたワールド行列を作る。
	/// </summary>
	Matrix4x4 MakeWorldMatrix() const;
};

} // namespace KujataEngine
