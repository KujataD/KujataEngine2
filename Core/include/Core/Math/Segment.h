#pragma once

#include "Core/Math/Vector3.h"

namespace KujataEngine {

/// <summary>
/// 始点と終点までの差分で表す線分。
/// </summary>
class Segment {
public:
	Vector3 origin; // 始点
	Vector3 diff;   // 終点の差分ベクトル
};

} // namespace KujataEngine
