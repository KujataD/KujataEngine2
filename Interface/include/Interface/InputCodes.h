#pragma once

namespace KujataEngine {

/// <summary>
/// キーボードのキーで、Windowsのキーコードへの変換はInputモジュールが持つ。
/// </summary>
enum class Key {
	A,
	B,
	C,
	D,
	E,
	F,
	G,
	H,
	I,
	J,
	K,
	L,
	M,
	N,
	O,
	P,
	Q,
	R,
	S,
	T,
	U,
	V,
	W,
	X,
	Y,
	Z,
	Num0,
	Num1,
	Num2,
	Num3,
	Num4,
	Num5,
	Num6,
	Num7,
	Num8,
	Num9,
	Space,
	Enter,
	Escape,
	Tab,
	Backspace,
	Shift,
	Control,
	Alt,
	Up,
	Down,
	Left,
	Right,
	// 数を表す印で、キーとしては使わない。
	Count,
};

/// <summary>
/// マウスのボタン。
/// </summary>
enum class MouseButton {
	Left,
	Right,
	Middle,
	// 数を表す印で、ボタンとしては使わない。
	Count,
};

/// <summary>
/// パッドのボタンで、トリガーは半分より押したら押したとみなす。
/// </summary>
enum class PadButton {
	A,
	B,
	X,
	Y,
	LeftShoulder,
	RightShoulder,
	LeftTrigger,
	RightTrigger,
	Back,
	Start,
	LeftThumb,
	RightThumb,
	DpadUp,
	DpadDown,
	DpadLeft,
	DpadRight,
	// 数を表す印で、ボタンとしては使わない。
	Count,
};

/// <summary>
/// パッドのスティック。
/// </summary>
enum class PadStick {
	Left,
	Right,
	// 数を表す印で、スティックとしては使わない。
	Count,
};

} // namespace KujataEngine
