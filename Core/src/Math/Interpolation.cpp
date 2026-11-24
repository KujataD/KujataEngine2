#include "Core/Math/Interpolation.h"

#include <algorithm>
#include <cmath>

namespace KujataEngine {

float Lerp(float f1, float f2, float t) { return f1 + (f2 - f1) * t; }

Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t) {
	return {
	    v1.x + (v2.x - v1.x) * t,
	    v1.y + (v2.y - v1.y) * t,
	    v1.z + (v2.z - v1.z) * t,
	};
}

Vector3 Slerp(const Vector3& a, const Vector3& b, float t) {
	Vector3 na = Normalize(a);
	Vector3 nb = Normalize(b);

	// cosθを求める
	float dot = Dot(na, nb);

	// 範囲外阻止
	dot = std::clamp(dot, -1.0f, 1.0f);

	// na->nbの角度θ
	float theta = std::acos(dot);

	// 角度がほぼ0（同じ方向）ならLerpで代用
	if (theta < 0.0001f) {
		return Normalize(Lerp(na, nb, t));
	}

	float sinTheta = std::sin(theta);

	// naのウェイト
	float w1 = std::sin((1.0f - t) * theta) / sinTheta;
	// nbのウェイト
	float w2 = std::sin(t * theta) / sinTheta;

	return na * w1 + nb * w2;
}

Vector3 Bezier(const Vector3& p0, const Vector3& p1, const Vector3& p2, float t) {
	Vector3 a = Lerp(p0, p1, t);
	Vector3 b = Lerp(p1, p2, t);
	return Lerp(a, b, t);
}

} // namespace KujataEngine
