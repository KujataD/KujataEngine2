#pragma once

#include "SceneGraph/Component.h"

/// <summary>
/// 弾に当たると、自分と弾を消す敵。
/// </summary>
class Enemy : public KujataEngine::Component {
public:
	void Update() override;
};
