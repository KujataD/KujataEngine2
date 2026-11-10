#include "InputDevices.h"

#include <Windows.h>
#include <Xinput.h>

#include <algorithm>

// XInputGetStateの実体が入っている。
#pragma comment(lib, "Xinput.lib")

namespace KujataEngine {

// キーの状態の上位のビットが立っていれば押している。
static const BYTE kKeyDownBit = 0x80;

// トリガーは0から255で、半分より押したらボタンとして扱う。
static const BYTE kTriggerThreshold = 128;

// スティックの値の最大で、これで割ると-1から1になる。
static const float kStickMax = 32767.0f;

/// <summary>
/// KeyをWindowsの仮想キーコードに変換する。
/// </summary>
static int ToVirtualKey(Key key) {
	const int index = static_cast<int>(key);

	// AからZと0から9は、仮想キーコードが文字の番号と同じ。
	if (key <= Key::Z) {
		return 'A' + index;
	}
	if (key <= Key::Num9) {
		return '0' + (index - static_cast<int>(Key::Num0));
	}

	switch (key) {
	case Key::Space:
		return VK_SPACE;
	case Key::Enter:
		return VK_RETURN;
	case Key::Escape:
		return VK_ESCAPE;
	case Key::Tab:
		return VK_TAB;
	case Key::Backspace:
		return VK_BACK;
	case Key::Shift:
		return VK_SHIFT;
	case Key::Control:
		return VK_CONTROL;
	case Key::Alt:
		return VK_MENU;
	case Key::Up:
		return VK_UP;
	case Key::Down:
		return VK_DOWN;
	case Key::Left:
		return VK_LEFT;
	case Key::Right:
		return VK_RIGHT;
	default:
		return 0;
	}
}

/// <summary>
/// MouseButtonをWindowsの仮想キーコードに変換する。
/// </summary>
static int ToVirtualKey(MouseButton button) {
	switch (button) {
	case MouseButton::Left:
		return VK_LBUTTON;
	case MouseButton::Right:
		return VK_RBUTTON;
	case MouseButton::Middle:
		return VK_MBUTTON;
	default:
		return 0;
	}
}

/// <summary>
/// トリガー以外のPadButtonを、XInputのボタンのビットに変換する。
/// </summary>
static WORD ToXInputBit(PadButton button) {
	switch (button) {
	case PadButton::A:
		return XINPUT_GAMEPAD_A;
	case PadButton::B:
		return XINPUT_GAMEPAD_B;
	case PadButton::X:
		return XINPUT_GAMEPAD_X;
	case PadButton::Y:
		return XINPUT_GAMEPAD_Y;
	case PadButton::LeftShoulder:
		return XINPUT_GAMEPAD_LEFT_SHOULDER;
	case PadButton::RightShoulder:
		return XINPUT_GAMEPAD_RIGHT_SHOULDER;
	case PadButton::Back:
		return XINPUT_GAMEPAD_BACK;
	case PadButton::Start:
		return XINPUT_GAMEPAD_START;
	case PadButton::LeftThumb:
		return XINPUT_GAMEPAD_LEFT_THUMB;
	case PadButton::RightThumb:
		return XINPUT_GAMEPAD_RIGHT_THUMB;
	case PadButton::DpadUp:
		return XINPUT_GAMEPAD_DPAD_UP;
	case PadButton::DpadDown:
		return XINPUT_GAMEPAD_DPAD_DOWN;
	case PadButton::DpadLeft:
		return XINPUT_GAMEPAD_DPAD_LEFT;
	case PadButton::DpadRight:
		return XINPUT_GAMEPAD_DPAD_RIGHT;
	default:
		return 0;
	}
}

/// <summary>
/// スティックの生の値を、遊びを除いて長さ0から1の値に直す。
/// </summary>
static Vector2 NormalizeStick(SHORT rawX, SHORT rawY, SHORT rawDeadZone) {
	Vector2 stick((std::max)(rawX / kStickMax, -1.0f), (std::max)(rawY / kStickMax, -1.0f));
	const float deadZone = rawDeadZone / kStickMax;

	const float length = Vector2::Length(stick);
	if (length <= deadZone) {
		return Vector2();
	}

	// 遊びの端から最大までを、0から1に広げ直す。
	const float scaledLength = (std::min)((length - deadZone) / (1.0f - deadZone), 1.0f);
	return stick * (scaledLength / length);
}

DeviceSnapshot InputDevices::Read(void* windowHandle) {
	DeviceSnapshot snapshot;

	// 他のウィンドウを使っている間に、キーが押されたままになるのとパッドで裏から動くのを防ぐ。
	if (GetForegroundWindow() != static_cast<HWND>(windowHandle)) {
		hasPreviousMousePosition_ = false;
		return snapshot;
	}

	ReadKeyboardAndMouseButtons(&snapshot);
	ReadMouse(windowHandle, &snapshot);
	ReadPad(&snapshot);
	return snapshot;
}

void InputDevices::ReadKeyboardAndMouseButtons(DeviceSnapshot* snapshot) {
	// メッセージの処理で更新される状態なので、他のウィンドウへの入力は入らない。
	BYTE keyStates[256] = {};
	if (!GetKeyboardState(keyStates)) {
		return;
	}

	for (int i = 0; i < static_cast<int>(Key::Count); i++) {
		snapshot->keys[i] = (keyStates[ToVirtualKey(static_cast<Key>(i))] & kKeyDownBit) != 0;
	}
	for (int i = 0; i < static_cast<int>(MouseButton::Count); i++) {
		snapshot->mouseButtons[i] = (keyStates[ToVirtualKey(static_cast<MouseButton>(i))] & kKeyDownBit) != 0;
	}
}

void InputDevices::ReadMouse(void* windowHandle, DeviceSnapshot* snapshot) {
	POINT cursor = {};
	if (!GetCursorPos(&cursor) || !ScreenToClient(static_cast<HWND>(windowHandle), &cursor)) {
		return;
	}

	const Vector2 position(static_cast<float>(cursor.x), static_cast<float>(cursor.y));
	snapshot->mousePosition = position;

	// 前面に戻った直後は前の位置がないので、動きを0にして飛びを防ぐ。
	if (hasPreviousMousePosition_) {
		snapshot->mouseMove = position - previousMousePosition_;
	}
	previousMousePosition_ = position;
	hasPreviousMousePosition_ = true;
}

void InputDevices::ReadPad(DeviceSnapshot* snapshot) {
	XINPUT_STATE state = {};
	if (XInputGetState(0, &state) != ERROR_SUCCESS) {
		return;
	}

	const XINPUT_GAMEPAD& pad = state.Gamepad;
	for (int i = 0; i < static_cast<int>(PadButton::Count); i++) {
		snapshot->padButtons[i] = (pad.wButtons & ToXInputBit(static_cast<PadButton>(i))) != 0;
	}
	snapshot->padButtons[static_cast<int>(PadButton::LeftTrigger)] = pad.bLeftTrigger >= kTriggerThreshold;
	snapshot->padButtons[static_cast<int>(PadButton::RightTrigger)] = pad.bRightTrigger >= kTriggerThreshold;

	snapshot->padSticks[static_cast<int>(PadStick::Left)] = NormalizeStick(pad.sThumbLX, pad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
	snapshot->padSticks[static_cast<int>(PadStick::Right)] = NormalizeStick(pad.sThumbRX, pad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
}

} // namespace KujataEngine
