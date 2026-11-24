#pragma once

#include "Core/Math/Vector3.h"

namespace KujataEngine {

// 角度を-πからπの範囲に収める。
float WrapAngle(float angle);

// 近い回り方でaからbへ補間する。
float LerpShortAngle(float a, float b, float t);

// 進む向きを向く、X軸とY軸の回転を返す。
Vector3 LookAt(const Vector3& velocity);

} // namespace KujataEngine
