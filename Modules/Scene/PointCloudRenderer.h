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

	/// <summary>
	/// 集まる先を差し替え、点は今の位置から新しい形へ流れていく。
	/// </summary>
	void SetTargets(PointCloudHandle targets);

	// 点の板の中心から端までの長さ。
	float pointSize = 0.01f;

	// 点1つが足す明るさで、点が重なる数だけ足されるので、密な点群ほど小さくする。
	float intensity = 0.1f;

private:
	PointCloudHandle targets_;

	// 描画の担当が点の位置と速度を覚えるための、このコンポーネントだけの番号。
	uint64_t instanceId_ = 0;
};

} // namespace KujataEngine
