#pragma once

#include "Core/IModule.h"
#include "Interface/IWindow.h"

#include <vector>

namespace KujataEngine {

/// <summary>
/// OSのウィンドウを作り、届いたメッセージを毎フレーム処理するモジュール。
/// </summary>
class Window : public IModule, public IWindow {
public:
	Window(const wchar_t* title, int clientWidth, int clientHeight);

	std::vector<std::string> GetProvidedServices() override;

	void Startup(ServiceRegistry& services, FrameLoop& frameLoop) override;
	void Shutdown() override;

	void* GetNativeHandle() override;
	int GetClientWidth() override;
	int GetClientHeight() override;
	void AddMessageHandler(std::function<bool(void* windowHandle, uint32_t message, uint64_t wparam, int64_t lparam)> handler) override;

	/// <summary>
	/// 登録された処理へ順にメッセージを渡し、どれかがtrueを返したらtrueを返す。
	/// </summary>
	bool DispatchToMessageHandlers(void* windowHandle, uint32_t message, uint64_t wparam, int64_t lparam);

private:
	void ProcessMessages();

	const wchar_t* title_;
	int clientWidth_;
	int clientHeight_;

	// Windows.hをこのヘッダーに持ち込まないように、HWNDをvoid*で持つ。
	void* handle_ = nullptr;
	FrameLoop* frameLoop_ = nullptr;

	// 窓より先にメッセージを受け取る処理の一覧。
	std::vector<std::function<bool(void* windowHandle, uint32_t message, uint64_t wparam, int64_t lparam)>> messageHandlers_;
};

} // namespace KujataEngine
