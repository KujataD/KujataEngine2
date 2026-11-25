#include "Bullet.h"

#include "Core/Math/Matrix4x4.h"
#include "SceneGraph/GameObject.h"
#include "Health.h"

using namespace KujataEngine;

Bullet::Bullet(ITime* time, const Vector3& velocity) {
	time_ = time;
	velocity_ = velocity;
}

void Bullet::OnTriggerEnter(KujataEngine::GameObject* other) {
	// 同じフレームで当たり済みの物は数えない。
	if (GetOwner()->IsDestroyed() || other->IsDestroyed()) {
		return;
	}

	if (Health* othersHealth = other->GetComponent<Health>()) {
		othersHealth->Damage(damage);
		GetOwner()->Destroy();
	}
}

void Bullet::Update() {
	const float deltaTime = time_->GetDeltaTime();
	GetOwner()->GetTransform().translation_ += velocity_ * deltaTime;

	aliveSeconds_ += deltaTime;
	if (aliveSeconds_ >= lifeSeconds) {
		GetOwner()->Destroy();
	}
}
