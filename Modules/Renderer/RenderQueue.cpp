#include "RenderQueue.h"

namespace KujataEngine {

void RenderQueue::SetCamera(const CameraData& camera) {
	camera_ = camera;
	hasCamera_ = true;
}

void RenderQueue::SubmitCube(const Matrix4x4& world) {
	cubeWorlds_.push_back(world);
}

bool RenderQueue::HasCamera() const {
	return hasCamera_;
}

const CameraData& RenderQueue::GetCamera() const {
	return camera_;
}

const std::vector<Matrix4x4>& RenderQueue::GetCubeWorlds() const {
	return cubeWorlds_;
}

void RenderQueue::Clear() {
	hasCamera_ = false;
	cubeWorlds_.clear();
}

} // namespace KujataEngine
