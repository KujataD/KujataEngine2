#pragma once

#include "Core/Math/MathUtil.h"
#include "Core/Math/Quaternion.h"

namespace KujataEngine {

/// <summary>
/// ワールド変換データ
/// </summary>
class WorldTransform {
public:
	// スケール・回転・平行移動
	Vector3 scale_ = {1.0f, 1.0f, 1.0f};
	Vector3 rotation_ = {0.0f, 0.0f, 0.0f};
	Vector3 translation_ = {0.0f, 0.0f, 0.0f};

	// ワールド行列（UpdateWorldMatrix後に有効）
	Matrix4x4 matWorld_ = MakeIdentity();

	// 親となるワールド変換へのポインタ（階層構造用）
	const WorldTransform* parent_ = nullptr;

	WorldTransform() = default;
	~WorldTransform() = default;

	/// <summary>
	/// 回転をQuaternionとして取得します(保存形式はEulerのまま)。
	/// </summary>
	Quaternion GetRotationQuaternion() const { return Quaternion::FromEuler(rotation_); }

	/// <summary>
	/// Quaternionの姿勢をEulerへ変換してrotation_に書き込みます。
	/// </summary>
	void SetRotationFromQuaternion(const Quaternion& rotation) { rotation_ = rotation.ToEuler(); }

	/// <summary>
	/// ワールド行列だけを更新する
	/// </summary>
	void UpdateWorldMatrix();

	void CalcRotationOfVelocity(const Vector3& velocity, const Vector3& deltaAngle = {0, 0, 0}, float maxRotationSpeed = 1.0f);

	Vector3 GetWorldPosition() const { return {matWorld_.m[3][0], matWorld_.m[3][1], matWorld_.m[3][2]}; }
	void SetWorldPosition(Vector3 worldPos) {
		if (parent_) {
			Matrix4x4 inverseParent = Inverse(parent_->matWorld_);
			Vector3 localPos = Transform(worldPos, inverseParent);

			translation_ = localPos;
		} else {
			translation_ = worldPos;
		}
	}

private:
	/// <summary>
	/// 親をたどってワールド行列を計算し、親の更新順に左右されないようにする。
	/// </summary>
	Matrix4x4 ComputeWorldMatrix() const;

	// コピー禁止
	WorldTransform(const WorldTransform&) = delete;
	WorldTransform& operator=(const WorldTransform&) = delete;
};

} // namespace KujataEngine
