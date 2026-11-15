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

	/// <summary>
	/// 次に描くフレームから全部の点を崩し、呼ぶたびに点ごとの散る位置と崩れる速さを変える。
	/// </summary>
	void Crumble();

	/// <summary>
	/// 次に描くフレームから、全部の点を集まる先へ集める。
	/// </summary>
	void GatherAll();

	// 点の板の中心から端までの長さ。
	float pointSize = 0.01f;

	// 点1つが足す明るさで、点が重なる数だけ足されるので、密な点群ほど小さくする。
	float intensity = 0.1f;

	// 最初に形になっている度合いで、0なら散った状態から、1なら形になった状態から始まる。
	float initialSettle = 1.0f;

	// 点が引き寄せる先のまわりを揺れ続ける幅で、0なら揺れない。
	float floatDistance = 0.05f;

	// 形になっていない点の明るさの倍率で、0にすると崩れた点が点ごとに違うタイミングで消える。
	float scatteredBrightness = 1.0f;

	// 描く点の割合で、1なら全部を描き、点は面にランダムに並んでいるので減らすと全体がまばらになる。
	float density = 1.0f;

private:
	PointCloudHandle targets_;

	// 次の依頼で渡す集める場所で、渡したら空にする。
	PointCloudGatherPoint gatherPoints_[kMaxPointCloudGatherPoints] = {};
	uint32_t gatherPointCount_ = 0;

	// 次の依頼で渡す形になる度合いの目標で、渡したら取り消す。
	bool hasSettleOverride_ = false;
	float settleOverride_ = 0.0f;

	// 崩すたびに増やす乱数の種で、毎回違う崩れ方にする。
	uint32_t scatterSeed_ = 0;

	// 描画の担当が点の位置と速度を覚えるための、このコンポーネントだけの番号。
	uint64_t instanceId_ = 0;
};

} // namespace KujataEngine
