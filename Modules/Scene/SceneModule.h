#pragma once

#include "Core/IModule.h"
#include "Interface/IRenderQueue.h"
#include "Framework/Scene.h"

namespace KujataEngine {

/// <summary>
/// Sceneを毎フレーム更新して、描きたい物を描画の担当へ渡すモジュール。
/// </summary>
class SceneModule : public IModule {
public:
	std::vector<std::string> GetProvidedServices() override;
	std::vector<std::string> GetRequiredServices() override;
	void Startup(ServiceRegistry& services, FrameLoop& frameLoop) override;

	Scene* GetScene();

private:
	Scene scene_;
	IRenderQueue* renderQueue_ = nullptr;
};

} // namespace KujataEngine
