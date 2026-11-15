#pragma once

#include "Core/Math/Matrix4x4.h"
#include "Core/Math/Vector3.h"
#include "Interface/AssetHandle.h"

#include <cstdint>

namespace KujataEngine {

/// <summary>
/// 描くときのカメラの情報で、画面の縦横比は描く側が決める。
/// </summary>
struct CameraData {
	Matrix4x4 view;
	float fovYRadians;
	float nearZ;
	float farZ;
};

/// <summary>
/// 平行光源の情報で、directionは光が進む向きを表す長さ1のベクトル。
/// </summary>
struct DirectionalLightData {
	Vector3 direction;
	Vector3 color;
	float intensity;
};

/// <summary>
/// 点を集める場所1つ分で、PointCloudUpdate.CS.hlslのGatherPointと並びを合わせる。
/// </summary>
struct PointCloudGatherPoint {
	// ワールド座標での場所。
	Vector3 position;

	// この距離より近くに集まる先がある点が、形になり始める。
	float radius;
};

// 1つの点群に1フレームで渡せる集める場所の数。
const uint32_t kMaxPointCloudGatherPoints = 16;

/// <summary>
/// 点群を1つ描く依頼で、点ごとの位置と速度は描画の担当がinstanceIdごとに覚える。
/// </summary>
struct PointCloudRequest {
	PointCloudHandle targets;

	// 描く物ごとに違う番号で、同じ番号なら前のフレームの点の続きから動く。
	uint64_t instanceId = 0;

	Matrix4x4 world;

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

	// trueのフレームだけ、全部の点の形になる度合いの目標をsettleOverrideにする。
	bool hasSettleOverride = false;
	float settleOverride = 0.0f;

	// 点ごとの散る位置と崩れる速さを決める乱数の種で、変えると同じ点群でも違う崩れ方になる。
	uint32_t scatterSeed = 0;

	// このフレームで点を集める場所で、近くに集まる先がある点は形になったままになる。
	PointCloudGatherPoint gatherPoints[kMaxPointCloudGatherPoints] = {};
	uint32_t gatherPointCount = 0;
};

/// <summary>
/// 描きたい物があるコンポーネントが、描画の担当に描く物を頼むための窓口。
/// </summary>
class IRenderQueue {
public:
	virtual ~IRenderQueue() = default;

	/// <summary>
	/// このフレームで使うカメラを指定する。
	/// </summary>
	virtual void SetCamera(const CameraData& camera) = 0;

	/// <summary>
	/// このフレームで使う平行光源を指定する。
	/// </summary>
	virtual void SetDirectionalLight(const DirectionalLightData& light) = 0;

	/// <summary>
	/// メッシュを指定の画像とワールド行列で描くように頼む。
	/// </summary>
	virtual void Submit(MeshHandle mesh, TextureHandle texture, const Matrix4x4& world) = 0;

	/// <summary>
	/// 点群を動かして描くように頼む。
	/// </summary>
	virtual void SubmitPointCloud(const PointCloudRequest& request) = 0;
};

} // namespace KujataEngine
