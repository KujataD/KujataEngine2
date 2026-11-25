#include "Enemy.h"

#include "Core/Math/Noise.h"
#include "Health.h"

using namespace KujataEngine;

Enemy::Enemy(ITime* time, Weapon* weapon, GameObject* target, float noiseSeed) {
	time_ = time;
	weapon_ = weapon;
	target_ = target;
	noiseSeed_ = noiseSeed;
}

void Enemy::Update() {
	GameObject* owner = GetOwner();
	if (Health* health = owner->GetComponent<Health>()) {
		if (!health->IsAlive()) {
			owner->Destroy();
			return;
		}
	}

	// 倒れた相手は止まっているので、動かず撃たない。
	if (!target_->IsActive()) {
		return;
	}

	// 相手への水平な向きで、移動と弾の両方に使う。
	Vector3 toTarget = target_->GetTransform().translation_ - owner->GetTransform().translation_;
	toTarget.y = 0.0f;
	const float targetDistance = Length(toTarget);
	if (targetDistance < kMinAimDistance) {
		return;
	}
	const Vector3 aimDirection = Normalize(toTarget);

	Move(aimDirection, targetDistance);

	// Weaponの間隔ごとに撃ち続ける。
	weapon_->TryFire(aimDirection);
}

void Enemy::Move(const Vector3& aimDirection, float targetDistance) {
	const float deltaTime = time_->GetDeltaTime();
	elapsedSeconds_ += deltaTime;

	const float noiseTime = elapsedSeconds_ * wanderFrequency;
	const Vector3 wander = {
	    PerlinNoise(noiseTime, noiseSeed_),
	    0.0f,
	    PerlinNoise(noiseTime, noiseSeed_ + kAxisRowOffset),
	};

	// 止まる距離のヒステリシス
	if (isChasing_) {
		isChasing_ = targetDistance > keepDistance;
	} else {
		isChasing_ = targetDistance > keepDistance + resumeMargin;
	}
	const float chase = isChasing_ ? chaseSpeed : 0.0f;

	const Vector3 velocity = aimDirection * chase + wander * wanderSpeed;
	GetOwner()->GetTransform().translation_ += velocity * deltaTime;
}
