#pragma once

#include "Core/Math/Vector2.h"
#include "Interface/InputCodes.h"

#include <string>

namespace KujataEngine {

// ServiceRegistryに登録するときの名前。
const char* const kInputActionsServiceName = "InputActions";

/// <summary>
/// ゲームが操作をアクションの名前で読むための窓口で、どの機器から来たかは見せない。
/// </summary>
class IInputActions {
public:
	virtual ~IInputActions() = default;

	// アクションの登録は準備の段階で行う。

	/// <summary>
	/// 押しているかどうかを持つボタンのアクションを作る。
	/// </summary>
	virtual void AddButton(const std::string& name) = 0;

	/// <summary>
	/// 上と右を正とした2つの値を持つアクションを作る。
	/// </summary>
	virtual void AddAxis2D(const std::string& name) = 0;

	/// <summary>
	/// ボタンのアクションにキーを割り当てる。
	/// </summary>
	virtual void BindKey(const std::string& action, Key key) = 0;

	/// <summary>
	/// ボタンのアクションにマウスのボタンを割り当てる。
	/// </summary>
	virtual void BindMouseButton(const std::string& action, MouseButton button) = 0;

	/// <summary>
	/// ボタンのアクションにパッドのボタンを割り当てる。
	/// </summary>
	virtual void BindPadButton(const std::string& action, PadButton button) = 0;

	/// <summary>
	/// 2軸のアクションに上下左右の4つのキーを割り当て、斜めも長さ1に収める。
	/// </summary>
	virtual void BindKeys2D(const std::string& action, Key up, Key down, Key left, Key right) = 0;

	/// <summary>
	/// 2軸のアクションに、遊びを除いて-1から1にしたパッドのスティックを割り当てる。
	/// </summary>
	virtual void BindPadStick(const std::string& action, PadStick stick) = 0;

	/// <summary>
	/// 2軸のアクションに、前のフレームからのマウスの移動を下が正のピクセルで割り当てるので、スティックと同じアクションには混ぜない。
	/// </summary>
	virtual void BindMouseMove(const std::string& action) = 0;

	// ゲームは毎フレームここから読む。

	/// <summary>
	/// ボタンのアクションを押しているかを返す。
	/// </summary>
	virtual bool IsHeld(const std::string& action) = 0;

	/// <summary>
	/// ボタンのアクションをこのフレームで押し始めたときだけtrueを返す。
	/// </summary>
	virtual bool IsPressed(const std::string& action) = 0;

	/// <summary>
	/// ボタンのアクションをこのフレームで離したときだけtrueを返す。
	/// </summary>
	virtual bool IsReleased(const std::string& action) = 0;

	/// <summary>
	/// 2軸のアクションの値を返し、割り当てが複数あればいちばん大きく入っているものを使う。
	/// </summary>
	virtual Vector2 GetAxis2D(const std::string& action) = 0;

	/// <summary>
	/// ウィンドウの中の左上を0とし、下を正としたマウスの位置をピクセルで返す。
	/// </summary>
	virtual Vector2 GetMousePosition() = 0;
};

} // namespace KujataEngine
