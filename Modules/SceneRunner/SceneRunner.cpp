#include "SceneRunner.h"

namespace KujataEngine {

std::vector<std::string> SceneRunner::GetProvidedServices() {
	return { kSceneServiceName };
}

void SceneRunner::Startup(ServiceRegistry& services, FrameLoop& frameLoop) {
	services.Register(kSceneServiceName, &scene_);

	frameLoop.Register(kUpdate, [this]() {
		scene_.Update();
	});

	// 物理が終わった後に消す物を消し、描画の前に行列を今の位置に合わせる。
	frameLoop.Register(kLateUpdate, [this]() {
		scene_.FlushDestroyed();
		scene_.UpdateWorldTransforms();
	});
}

Scene* SceneRunner::GetScene() {
	return &scene_;
}

} // namespace KujataEngine
