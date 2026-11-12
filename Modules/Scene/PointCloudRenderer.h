#pragma once

#include "Framework/Component.h"
#include "Interface/AssetHandle.h"
#include "Interface/IRenderQueue.h"
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

	/// <summary>
	/// 次に描くフレームで、ワールド座標の指定の場所の近くに集まる先がある点を形にする。
	/// </summary>
	void Gather(const Vector3& worldPosition, float radius);

	// 点の板の中心から端までの長さ。
	float pointSize = 0.01f;

	// 点1つが足す明るさで、点が重なる数だけ足されるので、密な点群ほど小さくする。
	float intensity = 0.1f;

	// 最初に形になっている度合いで、0なら散った状態から、1なら形になった状態から始まる。
	float initialSettle = 1.0f;

private:
	PointCloudHandle targets_;

	// 次の依頼で渡す集める場所で、渡したら空にする。
	PointCloudGatherPoint gatherPoints_[kMaxPointCloudGatherPoints] = {};
	uint32_t gatherPointCount_ = 0;

	// 描画の担当が点の位置と速度を覚えるための、このコンポーネントだけの番号。
	uint64_t instanceId_ = 0;
};

} // namespace KujataEngine
