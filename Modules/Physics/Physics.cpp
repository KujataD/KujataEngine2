#include "Physics.h"

#include "Core/Assert.h"
#include "ScenePhysics.h"

#include <algorithm>

namespace KujataEngine {

// 1回の物理の更新で進める時間で、単位は秒。
static const float kFixedDeltaTime = 1.0f / 60.0f;

// 処理が詰まった後に追いつこうとして、さらに遅くなるのを防ぐ上限で、単位は秒。
static const double kMaxAccumulatedTime = kFixedDeltaTime * 5.0;

std::vector<std::string> Physics::GetRequiredServices() {
	return { kSceneServiceName };
}

void Physics::Startup(ServiceRegistry& services, FrameLoop& frameLoop) {
	scene_ = static_cast<Scene*>(services.Get(kSceneServiceName));
	KUJATA_ASSERT(scene_ != nullptr);

	previousTime_ = std::chrono::steady_clock::now();
	frameLoop.Register(kUpdate, [this]() {
		Advance();
	});
}

void Physics::Advance() {
	const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	const double elapsedTime = std::chrono::duration<double>(now - previousTime_).count();
	previousTime_ = now;

	accumulatedTime_ = (std::min)(accumulatedTime_ + elapsedTime, kMaxAccumulatedTime);
	while (accumulatedTime_ >= kFixedDeltaTime) {
		Step();
		accumulatedTime_ -= kFixedDeltaTime;
	}
}

void Physics::Step() {
	ScenePhysics::IntegrateRigidbodies(*scene_, kFixedDeltaTime);

	// 動かした後の位置で衝突を調べ、押し出した結果を描画に反映する。
	scene_->UpdateWorldTransforms();
	collisionSystem_.Update(*scene_);
	scene_->UpdateWorldTransforms();
}

} // namespace KujataEngine
