#include "SpriteRenderer.h"

#include "Interface/IRenderQueue.h"
#include "SceneGraph/GameObject.h"
#include "SpriteQuad.h"

namespace KujataEngine {

SpriteRenderer::SpriteRenderer(MeshHandle quad, TextureHandle texture) {
	quad_ = quad;
	texture_ = texture;
}

void SpriteRenderer::SubmitRender(IRenderQueue* renderQueue) {
	const Matrix4x4 uvTransform = MakeSpriteUVTransform(uvOffset, uvScale, uvRotation);
	renderQueue->SubmitSprite(quad_, texture_, GetOwner()->GetTransform().matWorld_, color, uvTransform, sortingOrder);
}

} // namespace KujataEngine
