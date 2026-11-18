#pragma once

#include "Core/Math/Vector3.h"
#include "Framework/Component.h"
#include "Interface/ITime.h"

/// <summary>
/// まっすぐ飛び、決まった秒数がたつと消える弾。
/// </summary>
class Bullet : public KujataEngine::Component {
public:
	Bullet(KujataEngine::ITime* time, const KujataEngine::Vector3& velocity);

	void Update() override;

	// 撃ってから消えるまでの秒数。
	float lifeSeconds = 2.0f;

private:
	KujataEngine::ITime* time_;
	KujataEngine::Vector3 velocity_;
	float aliveSeconds_ = 0.0f;
};
