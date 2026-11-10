#pragma once

#include "Core/Math/Vector2.h"
#include "Interface/InputCodes.h"

namespace KujataEngine {

/// <summary>
/// 1フレーム分の機器の状態を、APIに依存しない形で持つ。
/// </summary>
struct DeviceSnapshot {
	bool keys[static_cast<int>(Key::Count)] = {};
	bool mouseButtons[static_cast<int>(MouseButton::Count)] = {};
	bool padButtons[static_cast<int>(PadButton::Count)] = {};

	// 遊びを除いて-1から1にした値で、上と右が正。
	Vector2 padSticks[static_cast<int>(PadStick::Count)];

	// ウィンドウの中の左上を0とし、下を正としたピクセル。
	Vector2 mousePosition;

	// 前のフレームからの移動で、単位はピクセル。
	Vector2 mouseMove;
};

/// <summary>
/// キーボードとマウスとXInputのパッドから、今の状態を読むクラス。
/// </summary>
class InputDevices {
public:
	/// <summary>
	/// 指定のウィンドウが前面にあるときだけ機器を読み、ないときはすべて離した状態を返す。
	/// </summary>
	DeviceSnapshot Read(void* windowHandle);

private:
	void ReadKeyboardAndMouseButtons(DeviceSnapshot* snapshot);
	void ReadMouse(void* windowHandle, DeviceSnapshot* snapshot);
	void ReadPad(DeviceSnapshot* snapshot);

	Vector2 previousMousePosition_;
	bool hasPreviousMousePosition_ = false;
};

} // namespace KujataEngine
