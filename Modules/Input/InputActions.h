#pragma once

#include "InputDevices.h"
#include "Interface/IInputActions.h"

#include <map>
#include <set>
#include <string>
#include <vector>

namespace KujataEngine {

/// <summary>
/// 上下左右の4つのキーで2軸を作る割り当て。
/// </summary>
struct KeyAxisBinding {
	Key up;
	Key down;
	Key left;
	Key right;
};

/// <summary>
/// 1つのアクションの割り当てと、今と前のフレームの値。
/// </summary>
struct InputAction {
	bool isAxis2D = false;

	std::vector<Key> keys;
	std::vector<MouseButton> mouseButtons;
	std::vector<PadButton> padButtons;
	std::vector<KeyAxisBinding> keyAxes;
	std::vector<PadStick> padSticks;
	bool usesMouseMove = false;

	bool isHeld = false;
	bool wasHeld = false;
	Vector2 axis;
};

/// <summary>
/// 登録されたアクションを、機器の状態から毎フレーム更新するクラス。
/// </summary>
class InputActions : public IInputActions {
public:
	void AddButton(const std::string& name) override;
	void AddAxis2D(const std::string& name) override;
	void BindKey(const std::string& action, Key key) override;
	void BindMouseButton(const std::string& action, MouseButton button) override;
	void BindPadButton(const std::string& action, PadButton button) override;
	void BindKeys2D(const std::string& action, Key up, Key down, Key left, Key right) override;
	void BindPadStick(const std::string& action, PadStick stick) override;
	void BindMouseMove(const std::string& action) override;

	bool IsHeld(const std::string& action) override;
	bool IsPressed(const std::string& action) override;
	bool IsReleased(const std::string& action) override;
	Vector2 GetAxis2D(const std::string& action) override;
	Vector2 GetMousePosition() override;

	/// <summary>
	/// 機器の状態から全アクションの値を更新し、毎フレームの最初に呼ぶ。
	/// </summary>
	void Update(const DeviceSnapshot& snapshot);

private:
	void AddAction(const std::string& name, bool isAxis2D);

	/// <summary>
	/// 指定の名前と種類のアクションを探し、なければログを1回出してnullptrを返す。
	/// </summary>
	InputAction* FindAction(const std::string& name, bool isAxis2D);

	/// <summary>
	/// 同じ内容のログは1回だけ出す。
	/// </summary>
	void WarnOnce(const std::string& message);

	std::map<std::string, InputAction> actions_;
	Vector2 mousePosition_;

	// 同じ間違いのログを毎フレーム出さないように、出した内容を覚える。
	std::set<std::string> warnedMessages_;
};

} // namespace KujataEngine
