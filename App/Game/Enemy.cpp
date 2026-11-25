#include "Enemy.h"

#include "Health.h"
#include "SceneGraph/GameObject.h"

using namespace KujataEngine;

void Enemy::Update() {
	// 死亡判定
	GameObject* owner = GetOwner();
	if (Health* health = owner->GetComponent<Health>()) {
		if (!health->IsAlive()) {
			owner->Destroy();
		}
	}
}
