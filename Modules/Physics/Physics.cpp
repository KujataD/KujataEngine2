#include "Physics.h"

#include "../Scene/SceneModule.h"
#include "Core/Assert.h"
#include "ScenePhysics.h"

namespace KujataEngine {

// 1回の物理の更新で進める時間で、単位は秒。
static const float kFixedDeltaTime = 1.0f / 60.0f;

std::vector<std::string> Physics::GetRequiredServices() {
	return { kSceneServiceName };
}

void Physics::Startup(ServiceRegistry& services, FrameLoop& frameLoop) {
	scene_ = static_cast<Scene*>(services.Get(kSceneServiceName));
	KUJATA_ASSERT(scene_ != nullptr);

	frameLoop.Register(kUpdate, [this]() {
		Step();
	});
}

void Physics::Step() {
	ScenePhysics::IntegrateRigidbodies(*scene_, kFixedDeltaTime);

	// 動かした後の位置で衝突を調べ、押し出した結果を描画に反映する。
	scene_->UpdateWorldTransforms();
	collisionSystem_.Update(*scene_);
	scene_->UpdateWorldTransforms();
}

} // namespace KujataEngine
