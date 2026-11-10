#pragma once

#include "Core/Math/Vector3.h"
#include "Framework/Component.h"

namespace KujataEngine {

/// <summary>
/// 押し出しと速度の反射で衝突に応える剛体で、これを持たないColliderは動かない壁になる。
/// </summary>
class RigidbodyComponent : public Component {
public:
	// 速度はゲームが与えるか重力と反射で変わり、摩擦や力を加える仕組みはまだない。
	const Vector3& GetVelocity() const { return velocity_; }
	void SetVelocity(const Vector3& velocity) { velocity_ = velocity; }

	// 反発係数[0,1]。0=完全非弾性、1=完全弾性。
	float GetBounciness() const { return bounciness_; }
	void SetBounciness(float bounciness) {
		if (bounciness < 0.0f) {
			bounciness = 0.0f;
		}
		if (bounciness > 1.0f) {
			bounciness = 1.0f;
		}
		bounciness_ = bounciness;
	}

	// 重力に掛ける倍率で、0で無重力、負で反重力になり、キネマティックには効かない。
	float GetGravityScale() const { return gravityScale_; }
	void SetGravityScale(float gravityScale) { gravityScale_ = gravityScale; }

	// trueのキネマティックは押し出されず重力も受けないが、自分の速度で動いて相手に運動量を渡す。
	bool IsStatic() const { return isStatic_; }
	void SetStatic(bool isStatic) { isStatic_ = isStatic; }

	// ワールドの軸ごとに速度を0にして、押し出されても動かないようにする。
	bool FreezePositionX() const { return freezePositionX_; }
	bool FreezePositionY() const { return freezePositionY_; }
	bool FreezePositionZ() const { return freezePositionZ_; }

	// ワールドの軸ごとの回転の固定で、角速度の物理がまだないので値を持つだけになる。
	bool FreezeRotationX() const { return freezeRotationX_; }
	bool FreezeRotationY() const { return freezeRotationY_; }
	bool FreezeRotationZ() const { return freezeRotationZ_; }

private:
	bool isStatic_ = false;
	float gravityScale_ = 1.0f;
	Vector3 velocity_ = {};
	float bounciness_ = 0.5f;
	bool freezePositionX_ = false;
	bool freezePositionY_ = false;
	bool freezePositionZ_ = false;
	bool freezeRotationX_ = false;
	bool freezeRotationY_ = false;
	bool freezeRotationZ_ = false;
};

} // namespace KujataEngine
