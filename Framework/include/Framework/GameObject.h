#pragma once

#include "Framework/Component.h"
#include "Framework/WorldTransform.h"

#include <cstdint>

#include <vector>

namespace KujataEngine {

/// <summary>
/// Componentを集めて持ち、毎フレームまとめて更新する入れ物。
/// </summary>
class GameObject {
public:
	/// <summary>
	/// 持っているComponentをすべてdeleteする。
	/// </summary>
	~GameObject();

	/// <summary>
	/// newで作ったComponentを受け取り、消す責任もGameObjectが持つ。
	/// </summary>
	void AddComponent(Component* component);

	/// <summary>
	/// 追加した順にComponentを更新し、途中で足されたComponentは次のフレームから動く。
	/// </summary>
	void Update();

	/// <summary>
	/// 全Componentの消える直前の処理を呼ぶ。
	/// </summary>
	void CallOnDestroy();

	/// <summary>
	/// 消す印を付け、実際に消すのはそのフレームの更新と物理が終わった後になる。
	/// </summary>
	void Destroy() { isDestroyed_ = true; }
	bool IsDestroyed() const { return isDestroyed_; }

	/// <summary>
	/// このGameObjectの位置と回転と大きさを返す。
	/// </summary>
	WorldTransform& GetTransform();
	const WorldTransform& GetTransform() const;

	/// <summary>
	/// 親をたどってワールド行列を今の位置に合わせる。
	/// </summary>
	void UpdateWorldTransformSelfAndAncestors();

	/// <summary>
	/// 指定の型のComponentを探し、なければnullptrを返す。
	/// </summary>
	template<typename T> T* GetComponent() {
		for (Component* component : components_) {
			T* found = dynamic_cast<T*>(component);
			if (found != nullptr) {
				return found;
			}
		}
		return nullptr;
	}

	std::vector<Component*>& GetComponents() { return components_; }

	void SetActive(bool isActive) { isActive_ = isActive; }
	bool IsActive() const { return isActive_; }

	// 衝突の絞り込みに使うLayerの番号。
	void SetLayer(uint32_t layer) { layer_ = layer; }
	uint32_t GetLayer() const { return layer_; }

private:
	WorldTransform transform_;
	bool isActive_ = true;
	bool isDestroyed_ = false;
	uint32_t layer_ = 0;
	std::vector<Component*> components_;
};

} // namespace KujataEngine
