#include "Enemy.h"

#include "Health.h"

using namespace KujataEngine;

Enemy::Enemy(Weapon* weapon, GameObject* target) {
	weapon_ = weapon;
	target_ = target;
}

void Enemy::Update() {
	GameObject* owner = GetOwner();
	if (Health* health = owner->GetComponent<Health>()) {
		if (!health->IsAlive()) {
			owner->Destroy();
			return;
		}
	}

	// 倒れた相手は止まっているので撃たない。
	if (!target_->IsActive()) {
		return;
	}

	// 相手への水平な向きへ、Weaponの間隔ごとに撃ち続ける。
	Vector3 toTarget = target_->GetTransform().translation_ - owner->GetTransform().translation_;
	toTarget.y = 0.0f;
	if (Length(toTarget) < kMinAimDistance) {
		return;
	}
	weapon_->TryFire(Normalize(toTarget));
}
