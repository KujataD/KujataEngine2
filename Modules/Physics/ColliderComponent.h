#pragma once

#include "Core/Math/Vector3.h"
#include "Framework/Component.h"
#include "Interface/ICollisionListener.h"
#include "AABB.h"
#include "ShapeUtil.h"
#include <cstdint>

namespace KujataEngine {

enum class ColliderShapeType {
	Sphere,
	Box,
	Capsule,
};

/// <summary>
/// GameObjectに付けるUnity風Colliderの基底Component。
/// </summary>
class ColliderComponent : public Component {
public:
	ColliderComponent();

	virtual ColliderShapeType GetShapeType() const = 0;

	bool IsTrigger() const { return isTrigger_; }

	/// <summary>
	/// 地面や足場として扱ってよいかを調べる側のために、このコライダーが動くかを返す。
	/// </summary>
	bool IsMovingCollider() const { return movingCollider_; }
	void SetMovingCollider(bool moving) { movingCollider_ = moving; }

	void SetTrigger(bool isTrigger) { isTrigger_ = isTrigger; }

	/// <summary>
	/// Collider中心のローカルオフセットを取得します。
	/// </summary>
	const Vector3& GetCenter() const { return center_; }

	void SetCenter(const Vector3& center) { center_ = center; }

	/// <summary>
	/// 接触対象Layerのビットマスクを取得します。
	/// </summary>
	uint32_t GetCollisionMask() const { return collisionMask_; }

	void SetCollisionMask(uint32_t collisionMask) { collisionMask_ = collisionMask; }

	/// <summary>
	/// Scene内のCollider識別子を取得します。
	/// </summary>
	uint64_t GetRuntimeId() const { return runtimeId_; }

	/// <summary>
	/// OwnerのLayerを衝突属性ビットとして取得します。
	/// </summary>
	uint32_t GetLayerMaskBit() const;

	/// <summary>
	/// 指定ColliderとLayer/Mask上接触可能かを返します。
	/// </summary>
	bool CanCollideWith(const ColliderComponent& other) const;

	/// <summary>
	/// Collider中心をワールド座標で取得します。
	/// </summary>
	Vector3 GetWorldCenter() const;

	virtual Sphere GetWorldSphere() const;

	virtual AABB GetWorldAABB() const;

	/// <summary>
	/// 指定Colliderと交差しているかを返します。
	/// </summary>
	bool Intersects(const ColliderComponent& other) const;

	/// <summary>
	/// 交差していれば自分から相手へ向かう法線で接触情報を埋めてtrueを返し、レイヤーの判定は呼び出す側が行う。
	/// </summary>
	bool ComputeContact(const ColliderComponent& other, Contact& out) const;

protected:
	bool isTrigger_ = false;
	// 部位ごとに分けた当たり判定のようにRigidbodyの有無からは推測できない構成があるので、置いた人が決める。
	bool movingCollider_ = false;
	Vector3 center_ = {0.0f, 0.0f, 0.0f};
	uint32_t collisionMask_ = 0xffffffff;

private:
	uint64_t runtimeId_ = 0;
};

class SphereColliderComponent : public ColliderComponent {
public:
	ColliderShapeType GetShapeType() const override { return ColliderShapeType::Sphere; }

	float GetRadius() const { return radius_; }
	void SetRadius(float radius);

	Sphere GetWorldSphere() const override;
	AABB GetWorldAABB() const override;

private:
	float radius_ = 0.5f;
};

/// <summary>
/// 箱Collider。WorldRotationがゼロならAABB、回転があればOBBとして扱います。
/// </summary>
class BoxColliderComponent : public ColliderComponent {
public:
	ColliderShapeType GetShapeType() const override { return ColliderShapeType::Box; }

	const Vector3& GetSize() const { return size_; }
	void SetSize(const Vector3& size);

	Sphere GetWorldSphere() const override;
	AABB GetWorldAABB() const override;

	/// <summary>
	/// WorldRotationがあるためOBBとして扱うかを返します。
	/// </summary>
	bool UsesWorldOBB() const;

	OBB GetWorldOBB() const;

private:
	Vector3 size_ = {1.0f, 1.0f, 1.0f};
};

/// <summary>
/// 半径と全長と軸の向きで形を決める、カプセルのCollider。
/// </summary>
class CapsuleColliderComponent : public ColliderComponent {
public:
	ColliderShapeType GetShapeType() const override { return ColliderShapeType::Capsule; }

	float GetRadius() const { return radius_; }
	void SetRadius(float radius);

	float GetHeight() const { return height_; }
	void SetHeight(float height);

	/// <summary>
	/// カプセル軸方向(0=X,1=Y,2=Z)。
	/// </summary>
	int GetDirection() const { return direction_; }
	void SetDirection(int direction);

	/// <summary>
	/// ワールド空間のカプセル(spine端点+半径)を返します。
	/// </summary>
	Capsule GetWorldCapsule() const;

	Sphere GetWorldSphere() const override;
	AABB GetWorldAABB() const override;

private:
	float radius_ = 0.5f;
	// 両端の半球を含む全長で、円柱の部分はここから半径の2倍を引いた長さになる。
	float height_ = 2.0f;
	int direction_ = 1; // 既定はY軸(Unity準拠)
};

} // namespace KujataEngine
