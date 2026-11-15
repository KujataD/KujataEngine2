#pragma once

#include <cstdint>
#include <functional>

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

	/// <summary>
	/// OSから届いたメッセージを窓より先に受け取る処理を登録し、処理がtrueを返したメッセージは窓で処理しない。
	/// </summary>
	virtual void AddMessageHandler(std::function<bool(void* windowHandle, uint32_t message, uint64_t wparam, int64_t lparam)> handler) = 0;
};

} // namespace KujataEngine
