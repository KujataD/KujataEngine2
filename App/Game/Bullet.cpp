#include "Bullet.h"

#include "Core/Math/MathUtil.h"
#include "Framework/GameObject.h"

using namespace KujataEngine;

Bullet::Bullet(ITime* time, const Vector3& velocity) {
	time_ = time;
	velocity_ = velocity;
}

void Bullet::Update() {
	const float deltaTime = time_->GetDeltaTime();
	GetOwner()->GetTransform().translation_ += velocity_ * deltaTime;

	aliveSeconds_ += deltaTime;
	if (aliveSeconds_ >= lifeSeconds) {
		GetOwner()->Destroy();
	}
}
