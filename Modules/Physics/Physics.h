#pragma once

#include "../Scene/Scene.h"
#include "Core/IModule.h"
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
	void Step();

	Scene* scene_ = nullptr;
	SceneCollisionSystem collisionSystem_;
};

} // namespace KujataEngine
