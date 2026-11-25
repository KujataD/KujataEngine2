#pragma once

#include "SceneGraph/Component.h"
#include "SceneGraph/GameObject.h"
#include "Weapon.h"

/// <summary>
/// 狙う相手へ撃ち続け、HPがなくなると消える敵。
/// </summary>
class Enemy : public KujataEngine::Component {
public:
	Enemy(Weapon* weapon, KujataEngine::GameObject* target);

	void Update() override;

private:
	// 相手がこれより近いと向きが決まらない。
	static constexpr float kMinAimDistance = 0.01f;

	Weapon* weapon_;
	KujataEngine::GameObject* target_;
};
