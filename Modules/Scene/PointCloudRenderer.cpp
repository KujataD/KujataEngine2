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

void PointCloudRenderer::SubmitRender(IRenderQueue* renderQueue) {
	PointCloudRequest request;
	request.targets = targets_;
	request.instanceId = instanceId_;
	request.world = GetOwner()->GetTransform().matWorld_;
	request.pointSize = pointSize;
	request.intensity = intensity;
	renderQueue->SubmitPointCloud(request);
}

} // namespace KujataEngine
