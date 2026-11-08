#include "MeshRenderer.h"

#include "Framework/GameObject.h"
#include "Interface/IRenderQueue.h"

namespace KujataEngine {

void MeshRenderer::SubmitRender(IRenderQueue* renderQueue) {
	renderQueue->SubmitCube(GetOwner()->GetTransform().matWorld_);
}

} // namespace KujataEngine
