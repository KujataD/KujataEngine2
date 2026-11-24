#include "ParticleRenderer.h"

#include "SceneGraph/GameObject.h"
#include "Interface/IRenderQueue.h"

namespace KujataEngine {

// 次に作るコンポーネントに渡す番号で、作るたびに増やして重ならないようにする。
static uint64_t nextInstanceId = 1;

ParticleRenderer::ParticleRenderer(IParticleRenderer* particleRenderer, ParticleShapeHandle shape) {
	particleRenderer_ = particleRenderer;
	shape_ = shape;
	instanceId_ = nextInstanceId;
	nextInstanceId++;
}

void ParticleRenderer::SetShape(ParticleShapeHandle shape) {
	// 粒の位置と速度は番号ごとに描画の担当が覚えているので、集まる先だけを替えればよい。
	shape_ = shape;
}

void ParticleRenderer::Gather(const Vector3& worldPosition, float radius) {
	// 1フレームに渡せる数を超えた分は、次のフレームでまた呼ばれる前提で捨てる。
	if (gatherPointCount_ >= kMaxParticleGatherPoints) {
		return;
	}

	gatherPoints_[gatherPointCount_].position = worldPosition;
	gatherPoints_[gatherPointCount_].radius = radius;
	gatherPointCount_++;
}

void ParticleRenderer::Crumble() {
	// 形になっている間に種を変えるので、散った粒がいきなり別の場所へ飛ぶことはない。
	scatterSeed_++;
	hasSettleOverride_ = true;
	settleOverride_ = 0.0f;
}

void ParticleRenderer::GatherAll() {
	hasSettleOverride_ = true;
	settleOverride_ = 1.0f;
}

void ParticleRenderer::SubmitRender(IRenderQueue* /*renderQueue*/) {
	ParticleRequest request;
	request.shape = shape_;
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
	particleRenderer_->SubmitParticles(request);
}

} // namespace KujataEngine
