#pragma once

#include "Core/Math/Vector3.h"

namespace KujataEngine {

// ポインタで持つだけなので、クラスの名前だけを伝える。
class GameObject;

/// <summary>
/// Collisionイベントで渡す接触情報。
/// </summary>
struct Collision {
	GameObject* self = nullptr;
	GameObject* other = nullptr;
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
	virtual void OnTriggerEnter(GameObject* other) {}
	virtual void OnTriggerStay(GameObject* other) {}
	virtual void OnTriggerExit(GameObject* other) {}
};

} // namespace KujataEngine
