#include "MeshRenderer.h"

#include "GameObject.h"
#include "Interface/IRenderQueue.h"

namespace KujataEngine {

void MeshRenderer::SubmitRender(IRenderQueue* renderQueue) {
	renderQueue->SubmitCube(GetOwner()->GetTransform().MakeWorldMatrix());
}

} // namespace KujataEngine
