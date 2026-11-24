#pragma once

#include "Core/Math/Vector3.h"
#include "SceneGraph/Component.h"
#include "SceneGraph/WorldTransform.h"

/// <summary>
/// 相手が動いた分だけ動き、相手との位置の差を保つ。
/// </summary>
class MovementFollower : public KujataEngine::Component {
public:
	MovementFollower(const KujataEngine::WorldTransform* target);

	void Start() override;
	void Update() override;

private:
	const KujataEngine::WorldTransform* target_;
	KujataEngine::Vector3 previousTargetPosition_;
};
