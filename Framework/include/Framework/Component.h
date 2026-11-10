#pragma once

namespace KujataEngine {

// GameObject.hをincludeするとお互いを読み合ってしまうので、クラスの名前だけを伝える。
class GameObject;

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
