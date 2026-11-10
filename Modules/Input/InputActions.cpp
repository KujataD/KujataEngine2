#include "InputActions.h"

#include <cstdio>

namespace KujataEngine {

/// <summary>
/// 割り当てのどれか1つでも押していればtrueを返す。
/// </summary>
static bool ReadButton(const InputAction& action, const DeviceSnapshot& snapshot) {
	for (Key key : action.keys) {
		if (snapshot.keys[static_cast<int>(key)]) {
			return true;
		}
	}
	for (MouseButton button : action.mouseButtons) {
		if (snapshot.mouseButtons[static_cast<int>(button)]) {
			return true;
		}
	}
	for (PadButton button : action.padButtons) {
		if (snapshot.padButtons[static_cast<int>(button)]) {
			return true;
		}
	}
	return false;
}

/// <summary>
/// 押しているキーを1、離しているキーを0にして返す。
/// </summary>
static float KeyValue(const DeviceSnapshot& snapshot, Key key) {
	return snapshot.keys[static_cast<int>(key)] ? 1.0f : 0.0f;
}

/// <summary>
/// 4つのキーから、上と右を正とした長さ1までの2軸を作る。
/// </summary>
static Vector2 ReadKeyAxis(const KeyAxisBinding& binding, const DeviceSnapshot& snapshot) {
	Vector2 axis(KeyValue(snapshot, binding.right) - KeyValue(snapshot, binding.left), KeyValue(snapshot, binding.up) - KeyValue(snapshot, binding.down));

	// 斜めは長さが1を超えるので、1に戻す。
	if (Vector2::Length(axis) > 1.0f) {
		axis = Vector2::Normalize(axis);
	}
	return axis;
}

/// <summary>
/// 割り当ての中から、いちばん大きく入っている2軸の値を返す。
/// </summary>
static Vector2 ReadAxis2D(const InputAction& action, const DeviceSnapshot& snapshot) {
	Vector2 strongest;

	for (const KeyAxisBinding& binding : action.keyAxes) {
		const Vector2 axis = ReadKeyAxis(binding, snapshot);
		if (Vector2::Length(axis) > Vector2::Length(strongest)) {
			strongest = axis;
		}
	}
	for (PadStick stick : action.padSticks) {
		const Vector2& axis = snapshot.padSticks[static_cast<int>(stick)];
		if (Vector2::Length(axis) > Vector2::Length(strongest)) {
			strongest = axis;
		}
	}
	if (action.usesMouseMove && Vector2::Length(snapshot.mouseMove) > Vector2::Length(strongest)) {
		strongest = snapshot.mouseMove;
	}
	return strongest;
}

void InputActions::AddButton(const std::string& name) {
	AddAction(name, false);
}

void InputActions::AddAxis2D(const std::string& name) {
	AddAction(name, true);
}

void InputActions::BindKey(const std::string& action, Key key) {
	InputAction* found = FindAction(action, false);
	if (found != nullptr) {
		found->keys.push_back(key);
	}
}

void InputActions::BindMouseButton(const std::string& action, MouseButton button) {
	InputAction* found = FindAction(action, false);
	if (found != nullptr) {
		found->mouseButtons.push_back(button);
	}
}

void InputActions::BindPadButton(const std::string& action, PadButton button) {
	InputAction* found = FindAction(action, false);
	if (found != nullptr) {
		found->padButtons.push_back(button);
	}
}

void InputActions::BindKeys2D(const std::string& action, Key up, Key down, Key left, Key right) {
	InputAction* found = FindAction(action, true);
	if (found != nullptr) {
		found->keyAxes.push_back({ up, down, left, right });
	}
}

void InputActions::BindPadStick(const std::string& action, PadStick stick) {
	InputAction* found = FindAction(action, true);
	if (found != nullptr) {
		found->padSticks.push_back(stick);
	}
}

void InputActions::BindMouseMove(const std::string& action) {
	InputAction* found = FindAction(action, true);
	if (found != nullptr) {
		found->usesMouseMove = true;
	}
}

bool InputActions::IsHeld(const std::string& action) {
	const InputAction* found = FindAction(action, false);
	if (found == nullptr) {
		return false;
	}
	return found->isHeld;
}

bool InputActions::IsPressed(const std::string& action) {
	const InputAction* found = FindAction(action, false);
	if (found == nullptr) {
		return false;
	}
	return found->isHeld && !found->wasHeld;
}

bool InputActions::IsReleased(const std::string& action) {
	const InputAction* found = FindAction(action, false);
	if (found == nullptr) {
		return false;
	}
	return !found->isHeld && found->wasHeld;
}

Vector2 InputActions::GetAxis2D(const std::string& action) {
	const InputAction* found = FindAction(action, true);
	if (found == nullptr) {
		return Vector2();
	}
	return found->axis;
}

Vector2 InputActions::GetMousePosition() {
	return mousePosition_;
}

void InputActions::Update(const DeviceSnapshot& snapshot) {
	mousePosition_ = snapshot.mousePosition;

	for (std::pair<const std::string, InputAction>& entry : actions_) {
		InputAction& action = entry.second;
		action.wasHeld = action.isHeld;

		if (action.isAxis2D) {
			action.axis = ReadAxis2D(action, snapshot);
		} else {
			action.isHeld = ReadButton(action, snapshot);
		}
	}
}

void InputActions::AddAction(const std::string& name, bool isAxis2D) {
	if (actions_.count(name) > 0) {
		WarnOnce("同じ名前のアクションがすでにあるので、追加しません: " + name);
		return;
	}

	InputAction action;
	action.isAxis2D = isAxis2D;
	actions_[name] = action;
}

InputAction* InputActions::FindAction(const std::string& name, bool isAxis2D) {
	if (actions_.count(name) == 0) {
		WarnOnce("登録していないアクションです: " + name);
		return nullptr;
	}

	InputAction& action = actions_[name];
	if (action.isAxis2D != isAxis2D) {
		WarnOnce("ボタンと2軸の種類が違うアクションです: " + name);
		return nullptr;
	}
	return &action;
}

void InputActions::WarnOnce(const std::string& message) {
	if (warnedMessages_.count(message) > 0) {
		return;
	}

	printf("%s\n", message.c_str());
	warnedMessages_.insert(message);
}

} // namespace KujataEngine
