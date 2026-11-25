#pragma once

#include "Core/Math/Vector3.h"
#include "Interface/ICollisionListener.h"
#include "Interface/ITime.h"
#include "SceneGraph/Component.h"

/// <summary>
/// まっすぐ飛び、決まった秒数がたつと消える弾。
/// </summary>
class Bullet : public KujataEngine::Component , public KujataEngine::ICollisionListener {
public:
	Bullet(KujataEngine::ITime* time, const KujataEngine::Vector3& velocity);

	void OnTriggerEnter(KujataEngine::GameObject* other) override;
	void Update() override;

	// 撃ってから消えるまでの秒数。
	float lifeSeconds = 2.0f;

	// 当たった相手から減らすHP。
	int damage = 4;

private:
	KujataEngine::ITime* time_;
	KujataEngine::Vector3 velocity_;
	float aliveSeconds_ = 0.0f;
};
