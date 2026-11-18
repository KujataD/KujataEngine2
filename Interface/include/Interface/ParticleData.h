#pragma once

#include "Core/Math/Matrix4x4.h"
#include "Core/Math/Vector3.h"

#include <cstdint>
#include <vector>

namespace KujataEngine {

/// <summary>
/// 登録した粒子の形を指す番号で、0番は代わりの1粒を表す。
/// </summary>
struct ParticleShapeHandle {
	uint32_t id = 0;
};

/// <summary>
/// 粒が集まる先1つ分で、ParticleShapeTable.cppの転送と粒子のシェーダーのParticleTargetと並びを合わせる。
/// </summary>
struct ParticleTarget {
	Vector3 position;
	Vector3 normal;
	Vector3 color;
};

/// <summary>
/// 粒が集まって作る形を、集まる先の一覧で表すデータで、GPUには依存しない。
/// </summary>
struct ParticleShape {
	std::vector<ParticleTarget> targets;
};

/// <summary>
/// 粒を集める場所1つ分で、ParticleSimulation.CS.hlslのGatherPointと並びを合わせる。
/// </summary>
struct ParticleGatherPoint {
	// ワールド座標での場所。
	Vector3 position;

	// この距離より近くに集まる先がある粒が、形になり始める。
	float radius;
};

// 1つの依頼で渡せる集める場所の数。
const uint32_t kMaxParticleGatherPoints = 16;

/// <summary>
/// 粒子を1つ動かして描く依頼で、粒ごとの位置と速度は描画の担当がinstanceIdごとに覚える。
/// </summary>
struct ParticleRequest {
	ParticleShapeHandle shape;

	// 描く物ごとに違う番号で、同じ番号なら前のフレームの粒の続きから動く。
	uint64_t instanceId = 0;

	Matrix4x4 world;

	// 粒の板の中心から端までの長さ。
	float pointSize = 0.01f;

	// 粒1つが足す明るさで、粒が重なる数だけ足されるので、密な形ほど小さくする。
	float intensity = 0.1f;

	// 最初に形になっている度合いで、0なら散った状態から、1なら形になった状態から始まる。
	float initialSettle = 1.0f;

	// 粒が引き寄せる先のまわりを揺れ続ける幅で、0なら揺れない。
	float floatDistance = 0.05f;

	// 形になっていない粒の明るさの倍率で、0にすると崩れた粒が粒ごとに違うタイミングで消える。
	float scatteredBrightness = 1.0f;

	// 描く粒の割合で、1なら全部を描き、粒は面にランダムに並んでいるので減らすと全体がまばらになる。
	float density = 1.0f;

	// trueのフレームだけ、全部の粒の形になる度合いの目標をsettleOverrideにする。
	bool hasSettleOverride = false;
	float settleOverride = 0.0f;

	// 粒ごとの散る位置と崩れる速さを決める乱数の種で、変えると同じ形でも違う崩れ方になる。
	uint32_t scatterSeed = 0;

	// このフレームで粒を集める場所で、近くに集まる先がある粒は形になったままになる。
	ParticleGatherPoint gatherPoints[kMaxParticleGatherPoints] = {};
	uint32_t gatherPointCount = 0;
};

} // namespace KujataEngine
