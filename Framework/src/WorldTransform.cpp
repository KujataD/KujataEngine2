#include "Framework/WorldTransform.h"

#include <cmath>

namespace KujataEngine {

Matrix4x4 WorldTransform::ComputeWorldMatrix() const {
	Matrix4x4 worldMatrix = MakeAffineMatrix(scale_, rotation_, translation_);
	if (parent_) {
		worldMatrix = worldMatrix * parent_->ComputeWorldMatrix();
	}
	return worldMatrix;
}

void WorldTransform::UpdateWorldMatrix() {
	matWorld_ = ComputeWorldMatrix();
}

void WorldTransform::CalcRotationOfVelocity(const Vector3& velocity, const Vector3& deltaAngle, float maxRotationSpeed) {
	// Y軸周り角度(θy) ...atan2(高さ, 底辺)
	float targetY = std::atan2(velocity.x, velocity.z);
	// 横軸方向の長さを求める
	float velocityXZ = Length({velocity.x, 0.0f, velocity.z});
	// X軸周り角度(θx)
	float targetX = std::atan2(-velocity.y, velocityXZ);

	if (std::abs(rotation_.x - targetX) > maxRotationSpeed) {
		rotation_.x += maxRotationSpeed;
	}

	rotation_.x = targetX;
	rotation_.y = targetY;

	rotation_ += deltaAngle;
}

} // namespace KujataEngine
