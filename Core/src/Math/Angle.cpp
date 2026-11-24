#include "Core/Math/Angle.h"

#include <cmath>
#include <numbers>

namespace KujataEngine {

Vector3 LookAt(const Vector3& velocity) {
	Vector3 rotation{};

	// Y軸周り角度(θy) ...atan2(高さ, 底辺)
	float targetY = std::atan2(velocity.x, velocity.z);
	// 横軸方向の長さを求める
	float velocityXZ = Length({velocity.x, 0.0f, velocity.z});
	// X軸周り角度(θx)
	float targetX = std::atan2(-velocity.y, velocityXZ);

	rotation.x = targetX;
	rotation.y = targetY;

	return rotation;
}

float WrapAngle(float angle) {

	while (angle > std::numbers::pi_v<float>) {
		angle -= std::numbers::pi_v<float> * 2.0f;
	}

	while (angle < -std::numbers::pi_v<float>) {
		angle += std::numbers::pi_v<float> * 2.0f;
	}

	return angle;
}

float LerpShortAngle(float a, float b, float t) {
	// 最短角度差
	float diff = WrapAngle(b - a);

	// 最短経路で補間
	return a + diff * t;
}

} // namespace KujataEngine
