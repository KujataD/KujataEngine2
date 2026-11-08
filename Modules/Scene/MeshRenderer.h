#pragma once

#include "Component.h"

namespace KujataEngine {

/// <summary>
/// 付いているGameObjectの位置に立方体を描くコンポーネント。
/// </summary>
class MeshRenderer : public Component {
public:
	void SubmitRender(IRenderQueue* renderQueue) override;
};

} // namespace KujataEngine
