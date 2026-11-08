#include "Camera.h"

#include "GameObject.h"
#include "Interface/IRenderQueue.h"

namespace KujataEngine {

/// <summary>
/// カメラの位置と回転を打ち消して、世界をカメラから見た形に直す行列を作る。
/// </summary>
static Matrix4x4 MakeViewMatrix(const Transform& transform) {
	const Matrix4x4 inverseTranslate = MakeTranslateMatrix(-transform.position.x, -transform.position.y, -transform.position.z);
	const Matrix4x4 inverseRotation = Multiply(Multiply(MakeRotationZMatrix(-transform.rotation.z), MakeRotationYMatrix(-transform.rotation.y)), MakeRotationXMatrix(-transform.rotation.x));
	return Multiply(inverseTranslate, inverseRotation);
}

void Camera::SubmitRender(IRenderQueue* renderQueue) {
	CameraData cameraData = {};
	cameraData.view = MakeViewMatrix(GetOwner()->GetTransform());
	cameraData.fovYRadians = fovYRadians;
	cameraData.nearZ = nearZ;
	cameraData.farZ = farZ;
	renderQueue->SetCamera(cameraData);
}

} // namespace KujataEngine
