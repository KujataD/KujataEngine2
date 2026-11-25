#include "RenderQueue.h"

namespace KujataEngine {

void RenderQueue::SetCamera(const CameraData& camera) {
	camera_ = camera;
	hasCamera_ = true;
}

void RenderQueue::SetDirectionalLight(const DirectionalLightData& light) {
	directionalLight_ = light;
	hasDirectionalLight_ = true;
}

void RenderQueue::Submit(MeshHandle mesh, TextureHandle texture, const Matrix4x4& world, const Vector4& tint) {
	DrawRequest request;
	request.mesh = mesh;
	request.texture = texture;
	request.world = world;
	request.tint = tint;
	drawRequests_.push_back(request);
}

void RenderQueue::SubmitSprite(MeshHandle quad, TextureHandle texture, const Matrix4x4& world, const Vector4& color, const Matrix4x4& uvTransform, int sortingOrder) {
	SpriteRequest request;
	request.quad = quad;
	request.texture = texture;
	request.world = world;
	request.color = color;
	request.uvTransform = uvTransform;
	request.sortingOrder = sortingOrder;
	spriteRequests_.push_back(request);
}

bool RenderQueue::HasCamera() const { return hasCamera_; }

const CameraData& RenderQueue::GetCamera() const { return camera_; }

bool RenderQueue::HasDirectionalLight() const { return hasDirectionalLight_; }

const DirectionalLightData& RenderQueue::GetDirectionalLight() const { return directionalLight_; }

const std::vector<DrawRequest>& RenderQueue::GetDrawRequests() const { return drawRequests_; }

const std::vector<SpriteRequest>& RenderQueue::GetSpriteRequests() const { return spriteRequests_; }

void RenderQueue::Clear() {
	hasCamera_ = false;
	hasDirectionalLight_ = false;
	drawRequests_.clear();
	spriteRequests_.clear();
}

} // namespace KujataEngine
