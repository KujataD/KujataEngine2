#pragma once

#include "Component.h"

#include <vector>

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

private:
	std::vector<Component*> components_;
};
