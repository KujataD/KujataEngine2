#pragma once

#include "SceneGraph/Component.h"
#include "Interface/ICollisionListener.h"

/// <summary>
/// 弾に当たると、自分と弾を消す敵。
/// </summary>
class Enemy : public KujataEngine::Component, public KujataEngine::ICollisionListener {
public:
	void OnTriggerEnter(KujataEngine::GameObject* other) override;
};
