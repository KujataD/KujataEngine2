#include "MeshRenderer.h"

#include "Framework/GameObject.h"
#include "Interface/IRenderQueue.h"

namespace KujataEngine {

MeshRenderer::MeshRenderer(MeshHandle mesh, TextureHandle texture) {
	mesh_ = mesh;
	texture_ = texture;
}

void MeshRenderer::SubmitRender(IRenderQueue* renderQueue) {
	renderQueue->Submit(mesh_, texture_, GetOwner()->GetTransform().matWorld_);
}

} // namespace KujataEngine
