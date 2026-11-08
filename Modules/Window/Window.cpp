#include "Window.h"

#include <Windows.h>
#include <cstdio>

namespace KujataEngine {

// ウィンドウの種類をOSに登録するときの名前。
static const wchar_t* const kWindowClassName = L"KujataEngine2Window";

/// <summary>
/// OSがウィンドウにメッセージを届けるたびに呼ばれる。
/// </summary>
static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
	// ウィンドウが閉じられたら、ProcessMessagesへWM_QUITを届ける。
	if (message == WM_DESTROY) {
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, message, wparam, lparam);
}

Window::Window(const wchar_t* title, int clientWidth, int clientHeight) {
	title_ = title;
	clientWidth_ = clientWidth;
	clientHeight_ = clientHeight;
}

void Window::Startup(ServiceRegistry& services, FrameLoop& frameLoop) {
	frameLoop_ = &frameLoop;

	HINSTANCE instance = GetModuleHandle(nullptr);

	WNDCLASS windowClass = {};
	windowClass.lpfnWndProc = WindowProc;
	windowClass.hInstance = instance;
	windowClass.lpszClassName = kWindowClassName;
	windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	RegisterClass(&windowClass);

	// 指定の大きさはタイトルバーと枠を除いた領域なので、枠を含めた大きさに直す。
	const DWORD windowStyle = WS_OVERLAPPEDWINDOW;
	RECT windowRect = { 0, 0, clientWidth_, clientHeight_ };
	AdjustWindowRect(&windowRect, windowStyle, FALSE);

	HWND hwnd = CreateWindow(
		kWindowClassName,
		title_,
		windowStyle,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr,
		nullptr,
		instance,
		nullptr);

	if (hwnd == nullptr) {
		printf("ウィンドウを作れませんでした: %lu\n", GetLastError());
		frameLoop.RequestQuit();
		return;
	}

	handle_ = hwnd;
	ShowWindow(hwnd, SW_SHOW);

	// 受け取る側がIWindow*に戻せるように、void*にする前にIWindow*へ変換する。
	services.Register("Window", static_cast<IWindow*>(this));

	frameLoop.Register(kBeginFrame, [this]() {
		ProcessMessages();
	});
}

void Window::Shutdown() {
	HWND hwnd = static_cast<HWND>(handle_);

	// ×で閉じたときはOSがもう消しているので、残っているときだけ消す。
	if (IsWindow(hwnd)) {
		DestroyWindow(hwnd);
	}

	UnregisterClass(kWindowClassName, GetModuleHandle(nullptr));
	handle_ = nullptr;
}

void* Window::GetNativeHandle() {
	return handle_;
}

int Window::GetClientWidth() {
	return clientWidth_;
}

int Window::GetClientHeight() {
	return clientHeight_;
}

void Window::ProcessMessages() {
	MSG message = {};
	while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE)) {
		if (message.message == WM_QUIT) {
			frameLoop_->RequestQuit();
		}

		TranslateMessage(&message);
		DispatchMessage(&message);
	}
}

} // namespace KujataEngine
