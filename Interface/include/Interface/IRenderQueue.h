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
/// 点群を1つ描く依頼で、点ごとの位置と速度は描画の担当がinstanceIdごとに覚える。
/// </summary>
struct PointCloudRequest {
	PointCloudHandle targets;

	// 描く物ごとに違う番号で、同じ番号なら前のフレームの点の続きから動く。
	uint64_t instanceId = 0;

	Matrix4x4 world;
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
