#pragma once

#include "Component.h"
#include "Transform.h"

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
	/// 追加した順にComponentを更新する。
	/// </summary>
	void Update();

	/// <summary>
	/// 追加した順にComponentへ描画の依頼を出させる。
	/// </summary>
	void SubmitRender(IRenderQueue* renderQueue);

	/// <summary>
	/// このGameObjectの位置と回転と大きさを返す。
	/// </summary>
	Transform& GetTransform();

private:
	Transform transform_;
	std::vector<Component*> components_;
};

} // namespace KujataEngine
