#pragma once

#include "Core/Math/Vector3.h"
#include "Framework/Component.h"
#include "Interface/IRenderable.h"

namespace KujataEngine {

/// <summary>
/// 付いているGameObjectの+Zの向きへ進む光で、シーン全体を同じ向きから照らすコンポーネント。
/// </summary>
class DirectionalLight : public Component, public IRenderable {
public:
	void SubmitRender(IRenderQueue* renderQueue) override;

	// 光の色で、各成分は0から1。
	Vector3 color = { 1.0f, 1.0f, 1.0f };

	// 色に掛ける明るさの倍率。
	float intensity = 1.0f;
};

} // namespace KujataEngine
