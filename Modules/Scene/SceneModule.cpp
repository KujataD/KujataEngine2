#include "SceneModule.h"

#include "Core/Assert.h"

namespace KujataEngine {

std::vector<std::string> SceneModule::GetRequiredServices() {
	return { kRenderQueueServiceName };
}

void SceneModule::Startup(ServiceRegistry& services, FrameLoop& frameLoop) {
	renderQueue_ = static_cast<IRenderQueue*>(services.Get(kRenderQueueServiceName));
	KUJATA_ASSERT(renderQueue_ != nullptr);

	frameLoop.Register(kUpdate, [this]() {
		scene_.Update();
	});
	frameLoop.Register(kSubmitRender, [this]() {
		scene_.SubmitRender(renderQueue_);
	});
}

Scene* SceneModule::GetScene() {
	return &scene_;
}

} // namespace KujataEngine
