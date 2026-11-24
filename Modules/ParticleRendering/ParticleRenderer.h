#pragma once

#include "SceneGraph/Component.h"
#include "Interface/IParticleRenderer.h"
#include "Interface/IRenderQueue.h"
#include "Interface/IRenderable.h"

#include <cstdint>

namespace KujataEngine {

/// <summary>
/// 付いているGameObjectの位置に、粒が集まって形になる粒子を描くコンポーネント。
/// </summary>
class ParticleRenderer : public Component, public IRenderable {
public:
	ParticleRenderer(IParticleRenderer* particleRenderer, ParticleShapeHandle shape);

	/// <summary>
	/// 描画の依頼を集める時に、粒子の窓口へ直接頼み、描画の依頼の入れ物は使わない。
	/// </summary>
	void SubmitRender(IRenderQueue* renderQueue) override;

	/// <summary>
	/// 形を差し替え、粒は今の位置から新しい形へ流れていく。
	/// </summary>
	void SetShape(ParticleShapeHandle shape);

	/// <summary>
	/// 次に描くフレームで、ワールド座標の指定の場所の近くに集まる先がある粒を形にする。
	/// </summary>
	void Gather(const Vector3& worldPosition, float radius);

	/// <summary>
	/// 次に描くフレームから全部の粒を崩し、呼ぶたびに粒ごとの散る位置と崩れる速さを変える。
	/// </summary>
	void Crumble();

	/// <summary>
	/// 次に描くフレームから、全部の粒を集まる先へ集める。
	/// </summary>
	void GatherAll();

	// 粒の板の中心から端までの長さ。
	float pointSize = 0.01f;

	// 粒1つが足す明るさで、粒が重なる数だけ足されるので、密な粒子ほど小さくする。
	float intensity = 0.1f;

	// 最初に形になっている度合いで、0なら散った状態から、1なら形になった状態から始まる。
	float initialSettle = 1.0f;

	// 粒が引き寄せる先のまわりを揺れ続ける幅で、0なら揺れない。
	float floatDistance = 0.05f;

	// 形になっていない粒の明るさの倍率で、0にすると崩れた粒が粒ごとに違うタイミングで消える。
	float scatteredBrightness = 1.0f;

	// 描く粒の割合で、1なら全部を描き、粒は面にランダムに並んでいるので減らすと全体がまばらになる。
	float density = 1.0f;

private:
	IParticleRenderer* particleRenderer_ = nullptr;
	ParticleShapeHandle shape_;

	// 次の依頼で渡す集める場所で、渡したら空にする。
	ParticleGatherPoint gatherPoints_[kMaxParticleGatherPoints] = {};
	uint32_t gatherPointCount_ = 0;

	// 次の依頼で渡す形になる度合いの目標で、渡したら取り消す。
	bool hasSettleOverride_ = false;
	float settleOverride_ = 0.0f;

	// 崩すたびに増やす乱数の種で、毎回違う崩れ方にする。
	uint32_t scatterSeed_ = 0;

	// 描画の担当が粒の位置と速度を覚えるための、このコンポーネントだけの番号。
	uint64_t instanceId_ = 0;
};

} // namespace KujataEngine
