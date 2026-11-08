#include "Camera.h"

#include "Framework/GameObject.h"
#include "Interface/IRenderQueue.h"

namespace KujataEngine {

void Camera::SubmitRender(IRenderQueue* renderQueue) {
	CameraData cameraData = {};
	// カメラの置かれた位置と向きを打ち消すと、世界をカメラから見た形になる。
	cameraData.view = Inverse(GetOwner()->GetTransform().matWorld_);
	cameraData.fovYRadians = fovYRadians;
	cameraData.nearZ = nearZ;
	cameraData.farZ = farZ;
	renderQueue->SetCamera(cameraData);
}

} // namespace KujataEngine
