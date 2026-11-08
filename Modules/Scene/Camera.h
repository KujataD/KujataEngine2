#pragma once

#include "Component.h"

namespace KujataEngine {

/// <summary>
/// 付いているGameObjectの位置と向きからシーンを写すコンポーネント。
/// </summary>
class Camera : public Component {
public:
	void SubmitRender(IRenderQueue* renderQueue) override;

	// 縦の視野角で、単位はラジアン。
	float fovYRadians = 0.7854f;

	// 写す範囲の手前と奥で、単位はワールド座標。
	float nearZ = 0.1f;
	float farZ = 100.0f;
};

} // namespace KujataEngine
