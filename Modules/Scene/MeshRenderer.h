#pragma once

#include "Framework/Component.h"
#include "Interface/IRenderable.h"

namespace KujataEngine {

/// <summary>
/// 付いているGameObjectの位置に立方体を描くコンポーネント。
/// </summary>
class MeshRenderer : public Component, public IRenderable {
public:
	void SubmitRender(IRenderQueue* renderQueue) override;
};

} // namespace KujataEngine
