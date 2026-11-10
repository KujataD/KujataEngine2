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
	/// 最初のUpdateの直前に1回だけ呼ばれる。
	/// </summary>
	virtual void Start() {}

	/// <summary>
	/// 毎フレーム1回呼ばれる。
	/// </summary>
	virtual void Update() {}

	/// <summary>
	/// 付いているGameObjectが消える直前に1回だけ呼ばれる。
	/// </summary>
	virtual void OnDestroy() {}

	/// <summary>
	/// まだStartを呼んでいなければ呼ぶ。
	/// </summary>
	void StartIfNeeded() {
		if (hasStarted_) {
			return;
		}
		hasStarted_ = true;
		Start();
	}

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
	bool hasStarted_ = false;
};

} // namespace KujataEngine
