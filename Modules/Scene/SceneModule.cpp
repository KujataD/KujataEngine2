#include "SceneModule.h"

namespace KujataEngine {

std::vector<std::string> SceneModule::GetProvidedServices() {
	return { kSceneServiceName };
}

void SceneModule::Startup(ServiceRegistry& services, FrameLoop& frameLoop) {
	services.Register(kSceneServiceName, &scene_);

	frameLoop.Register(kUpdate, [this]() {
		scene_.Update();
		scene_.UpdateWorldTransforms();
	});
}

Scene* SceneModule::GetScene() {
	return &scene_;
}

} // namespace KujataEngine
