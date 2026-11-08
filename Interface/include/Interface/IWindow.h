#pragma once

namespace KujataEngine {

// ServiceRegistryに登録するときの名前。
const char* const kWindowServiceName = "Window";

/// <summary>
/// OSのウィンドウを他のモジュールから使うための窓口。
/// </summary>
class IWindow {
public:
	virtual ~IWindow() = default;

	/// <summary>
	/// OSのウィンドウハンドルを返し、WindowsではHWNDとして使う。
	/// </summary>
	virtual void* GetNativeHandle() = 0;

	/// <summary>
	/// 描画できる領域の幅をピクセルで返す。
	/// </summary>
	virtual int GetClientWidth() = 0;

	/// <summary>
	/// 描画できる領域の高さをピクセルで返す。
	/// </summary>
	virtual int GetClientHeight() = 0;
};

} // namespace KujataEngine
