#include "Enemy.h"

#include "SceneGraph/GameObject.h"

using namespace KujataEngine;

void Enemy::OnTriggerEnter(GameObject* other) {
	// 同じフレームで当たり済みの物は数えない。
	if (GetOwner()->IsDestroyed() || other->IsDestroyed()) {
		return;
	}
	other->Destroy();
	GetOwner()->Destroy();
}
