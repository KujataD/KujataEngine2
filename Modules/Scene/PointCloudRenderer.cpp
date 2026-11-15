#include "PointCloudRenderer.h"

#include "Framework/GameObject.h"
#include "Interface/IRenderQueue.h"

namespace KujataEngine {

// 次に作るコンポーネントに渡す番号で、作るたびに増やして重ならないようにする。
static uint64_t nextInstanceId = 1;

PointCloudRenderer::PointCloudRenderer(PointCloudHandle targets) {
	targets_ = targets;
	instanceId_ = nextInstanceId;
	nextInstanceId++;
}

void PointCloudRenderer::SetTargets(PointCloudHandle targets) {
	// 点の位置と速度は番号ごとに描画の担当が覚えているので、集まる先だけを替えればよい。
	targets_ = targets;
}

void PointCloudRenderer::Gather(const Vector3& worldPosition, float radius) {
	// 1フレームに渡せる数を超えた分は、次のフレームでまた呼ばれる前提で捨てる。
	if (gatherPointCount_ >= kMaxPointCloudGatherPoints) {
		return;
	}

	gatherPoints_[gatherPointCount_].position = worldPosition;
	gatherPoints_[gatherPointCount_].radius = radius;
	gatherPointCount_++;
}

void PointCloudRenderer::Crumble() {
	// 形になっている間に種を変えるので、散った点がいきなり別の場所へ飛ぶことはない。
	scatterSeed_++;
	hasSettleOverride_ = true;
	settleOverride_ = 0.0f;
}

void PointCloudRenderer::GatherAll() {
	hasSettleOverride_ = true;
	settleOverride_ = 1.0f;
}

void PointCloudRenderer::SubmitRender(IRenderQueue* renderQueue) {
	PointCloudRequest request;
	request.targets = targets_;
	request.instanceId = instanceId_;
	request.world = GetOwner()->GetTransform().matWorld_;
	request.pointSize = pointSize;
	request.intensity = intensity;
	request.initialSettle = initialSettle;
	request.floatDistance = floatDistance;
	request.scatteredBrightness = scatteredBrightness;
	request.density = density;
	request.scatterSeed = scatterSeed_;

	// 目標の指定は1回だけ効かせたいので、渡したら取り消す。
	request.hasSettleOverride = hasSettleOverride_;
	request.settleOverride = settleOverride_;
	hasSettleOverride_ = false;

	for (uint32_t i = 0; i < gatherPointCount_; i++) {
		request.gatherPoints[i] = gatherPoints_[i];
	}
	request.gatherPointCount = gatherPointCount_;
	gatherPointCount_ = 0;
	renderQueue->SubmitPointCloud(request);
}

} // namespace KujataEngine
