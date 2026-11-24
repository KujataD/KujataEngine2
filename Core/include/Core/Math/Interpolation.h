#pragma once

#include "Core/Math/Vector3.h"

namespace KujataEngine {

float Lerp(float f1, float f2, float t);
Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t);

// 向きだけを球面に沿って補間し、長さ1で返す。
Vector3 Slerp(const Vector3& v1, const Vector3& v2, float t);

// p1を制御点にした2次のベジェ曲線。
Vector3 Bezier(const Vector3& p0, const Vector3& p1, const Vector3& p2, float t);

} // namespace KujataEngine
