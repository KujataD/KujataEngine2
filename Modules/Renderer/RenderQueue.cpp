#include "RenderQueue.h"

namespace KujataEngine {

void RenderQueue::SetCamera(const CameraData& camera) {
	camera_ = camera;
	hasCamera_ = true;
}

void RenderQueue::Submit(MeshHandle mesh, TextureHandle texture, const Matrix4x4& world) {
	DrawRequest request;
	request.mesh = mesh;
	request.texture = texture;
	request.world = world;
	drawRequests_.push_back(request);
}

bool RenderQueue::HasCamera() const {
	return hasCamera_;
}

const CameraData& RenderQueue::GetCamera() const {
	return camera_;
}

const std::vector<DrawRequest>& RenderQueue::GetDrawRequests() const {
	return drawRequests_;
}

void RenderQueue::Clear() {
	hasCamera_ = false;
	drawRequests_.clear();
}

} // namespace KujataEngine
