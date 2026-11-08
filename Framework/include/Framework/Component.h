#pragma once

namespace KujataEngine {

// GameObject.hをincludeするとお互いを読み合ってしまうので、クラスの名前だけを伝える。
class GameObject;
class IRenderQueue;
class ColliderComponent;
struct Collision;

/// <summary>
/// GameObjectに付ける機能のまとまりで、各コンポーネントはこれを継承する。
/// </summary>
class Component {
public:
	virtual ~Component() = default;

	/// <summary>
	/// 毎フレーム1回呼ばれる。
	/// </summary>
	virtual void Update() {}

	/// <summary>
	/// 描きたい物があるときに、毎フレーム描画の担当へ頼む。
	/// </summary>
	virtual void SubmitRender(IRenderQueue* renderQueue) {}

	// 非トリガーのColliderと接触したときに呼ばれる。
	virtual void OnCollisionEnter(const Collision& collision) {}
	virtual void OnCollisionStay(const Collision& collision) {}
	virtual void OnCollisionExit(const Collision& collision) {}

	// トリガーのColliderと重なったときに呼ばれる。
	virtual void OnTriggerEnter(ColliderComponent* other) {}
	virtual void OnTriggerStay(ColliderComponent* other) {}
	virtual void OnTriggerExit(ColliderComponent* other) {}

	/// <summary>
	/// Colliderのときだけ自分を返し、物理の処理がColliderを見分けるのに使う。
	/// </summary>
	virtual ColliderComponent* AsColliderComponent() { return nullptr; }

	void SetEnabled(bool isEnabled) { isEnabled_ = isEnabled; }
	bool IsEnabled() const { return isEnabled_; }

	/// <summary>
	/// 自分が付いているGameObjectを覚える。
	/// </summary>
	void SetOwner(GameObject* owner) {
		owner_ = owner;
	}

	/// <summary>
	/// 自分が付いているGameObjectを返す。
	/// </summary>
	GameObject* GetOwner() const {
		return owner_;
	}

private:
	GameObject* owner_ = nullptr;
	bool isEnabled_ = true;
};

} // namespace KujataEngine
