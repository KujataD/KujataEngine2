#include "DirectionalLight.h"

#include "Core/Math/MathUtil.h"
#include "Framework/GameObject.h"
#include "Interface/IRenderQueue.h"

namespace KujataEngine {

void DirectionalLight::SubmitRender(IRenderQueue* renderQueue) {
	DirectionalLightData lightData = {};
	// 拡大が入っていても向きだけを使うように、回した後に長さを1にする。
	lightData.direction = Normalize(TransformNormal({ 0.0f, 0.0f, 1.0f }, GetOwner()->GetTransform().matWorld_));
	lightData.color = color;
	lightData.intensity = intensity;
	renderQueue->SetDirectionalLight(lightData);
}

} // namespace KujataEngine
