#include "Physics.h"

#include "Core/Assert.h"
#include "ScenePhysics.h"

#include <algorithm>

namespace KujataEngine {

// 1回の物理の更新で進める時間で、単位は秒。
static const float kFixedDeltaTime = 1.0f / 60.0f;

// 処理が詰まった後に追いつこうとして、さらに遅くなるのを防ぐ上限で、単位は秒。
static const float kMaxAccumulatedTime = kFixedDeltaTime * 5.0f;

std::vector<std::string> Physics::GetRequiredServices() {
	return { kSceneServiceName, kTimeServiceName };
}

void Physics::Startup(ServiceRegistry& services, FrameLoop& frameLoop) {
	scene_ = static_cast<Scene*>(services.Get(kSceneServiceName));
	KUJATA_ASSERT(scene_ != nullptr);

	time_ = static_cast<ITime*>(services.Get(kTimeServiceName));
	KUJATA_ASSERT(time_ != nullptr);

	// 消える物と触れていた相手にExitを送り、消えた物を指したままにしない。
	scene_->AddDestroyListener([this](GameObject* gameObject) {
		collisionSystem_.ForgetGameObject(gameObject);
	});

	frameLoop.Register(kUpdate, [this]() {
		Advance();
	});
}

void Physics::Advance() {
	// 時間の速さを掛けた秒数を貯めるので、ゆっくりや停止も物理に効く。
	accumulatedTime_ = (std::min)(accumulatedTime_ + time_->GetDeltaTime(), kMaxAccumulatedTime);
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
