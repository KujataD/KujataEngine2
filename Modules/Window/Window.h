#pragma once

#include "Core/IModule.h"
#include "Interface/IWindow.h"

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

private:
	void ProcessMessages();

	const wchar_t* title_;
	int clientWidth_;
	int clientHeight_;

	// Windows.hをこのヘッダーに持ち込まないように、HWNDをvoid*で持つ。
	void* handle_ = nullptr;
	FrameLoop* frameLoop_ = nullptr;
};

} // namespace KujataEngine
