#pragma once

#include "Core/Math/Vector3.h"

namespace KujataEngine {

// ポインタで持つだけなので、クラスの名前だけを伝える。
class ColliderComponent;
class GameObject;

/// <summary>
/// Collisionイベントで渡す接触情報。
/// </summary>
struct Collision {
	ColliderComponent* self = nullptr;
	ColliderComponent* other = nullptr;
	GameObject* selfObject = nullptr;
	GameObject* otherObject = nullptr;
	Vector3 point = {};
	Vector3 normal = {};
	float penetrationDepth = 0.0f;
};

/// <summary>
/// 衝突の通知を受け取りたいコンポーネントが継承する窓口。
/// </summary>
class ICollisionListener {
public:
	virtual ~ICollisionListener() = default;

	// 非トリガーのColliderと接触したときに呼ばれる。
	virtual void OnCollisionEnter(const Collision& collision) {}
	virtual void OnCollisionStay(const Collision& collision) {}
	virtual void OnCollisionExit(const Collision& collision) {}

	// トリガーのColliderと重なったときに呼ばれる。
	virtual void OnTriggerEnter(ColliderComponent* other) {}
	virtual void OnTriggerStay(ColliderComponent* other) {}
	virtual void OnTriggerExit(ColliderComponent* other) {}
};

} // namespace KujataEngine
