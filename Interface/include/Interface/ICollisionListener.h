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

	/// <summary>
	/// 非トリガーのColliderと触れ始めたときに呼ばれる。
	/// </summary>
	virtual void OnCollisionEnter(const Collision& collision) {}

	/// <summary>
	/// 非トリガーのColliderと触れている間、当たり判定のたびに呼ばれる。
	/// </summary>
	virtual void OnCollisionStay(const Collision& collision) {}

	/// <summary>
	/// 非トリガーのColliderから離れたときに呼ばれる。
	/// </summary>
	virtual void OnCollisionExit(const Collision& collision) {}

	/// <summary>
	/// トリガーのColliderと重なり始めたときに呼ばれる。
	/// </summary>
	virtual void OnTriggerEnter(GameObject* other) {}

	/// <summary>
	/// トリガーのColliderと重なっている間、当たり判定のたびに呼ばれる。
	/// </summary>
	virtual void OnTriggerStay(GameObject* other) {}

	/// <summary>
	/// トリガーのColliderから離れたときに呼ばれる。
	/// </summary>
	virtual void OnTriggerExit(GameObject* other) {}
};

} // namespace KujataEngine
