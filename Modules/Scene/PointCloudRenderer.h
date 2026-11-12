#pragma once

#include "Framework/Component.h"
#include "Interface/AssetHandle.h"
#include "Interface/IRenderable.h"

#include <cstdint>

namespace KujataEngine {

/// <summary>
/// 付いているGameObjectの位置に、点が集まって形になる点群を描くコンポーネント。
/// </summary>
class PointCloudRenderer : public Component, public IRenderable {
public:
	PointCloudRenderer(PointCloudHandle targets);

	void SubmitRender(IRenderQueue* renderQueue) override;

private:
	PointCloudHandle targets_;

	// 描画の担当が点の位置と速度を覚えるための、このコンポーネントだけの番号。
	uint64_t instanceId_ = 0;
};

} // namespace KujataEngine
