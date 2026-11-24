#pragma once

#include "Core/IModule.h"
#include "SceneGraph/Scene.h"

namespace KujataEngine {

/// <summary>
/// Sceneを毎フレーム更新するモジュールで、描画がなくても動く。
/// </summary>
class SceneRunner : public IModule {
public:
	std::vector<std::string> GetProvidedServices() override;
	void Startup(ServiceRegistry& services, FrameLoop& frameLoop) override;

	Scene* GetScene();

private:
	Scene scene_;
};

} // namespace KujataEngine
