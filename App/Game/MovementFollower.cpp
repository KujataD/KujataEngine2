#include "MovementFollower.h"

#include "Core/Math/MathUtil.h"
#include "Framework/GameObject.h"

using namespace KujataEngine;

MovementFollower::MovementFollower(const WorldTransform* target) {
	target_ = target;
}

void MovementFollower::Start() {
	previousTargetPosition_ = target_->translation_;
}

void MovementFollower::Update() {
	// 親子にすると回転まで付いてくるので、位置の変化だけを足す。
	const Vector3 movement = target_->translation_ - previousTargetPosition_;
	GetOwner()->GetTransform().translation_ += movement;
	previousTargetPosition_ = target_->translation_;
}
