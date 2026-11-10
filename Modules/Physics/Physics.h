#pragma once

#include "Framework/Scene.h"
#include "Core/IModule.h"
#include "Interface/ITime.h"
#include "SceneCollisionSystem.h"

namespace KujataEngine {

/// <summary>
/// Sceneの剛体を動かして、衝突の検出と通知をするモジュール。
/// </summary>
class Physics : public IModule {
public:
	std::vector<std::string> GetRequiredServices() override;
	void Startup(ServiceRegistry& services, FrameLoop& frameLoop) override;

private:
	/// <summary>
	/// 経過時間を貯めて、1/60秒たまるごとにStepを1回進める。
	/// </summary>
	void Advance();

	void Step();

	Scene* scene_ = nullptr;
	ITime* time_ = nullptr;
	SceneCollisionSystem collisionSystem_;
	float accumulatedTime_ = 0.0f;
};

} // namespace KujataEngine
