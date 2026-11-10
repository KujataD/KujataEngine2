#pragma once

#include "Core/IModule.h"
#include "InputActions.h"
#include "InputDevices.h"
#include "Interface/IWindow.h"

namespace KujataEngine {

/// <summary>
/// 毎フレームの最初に機器を読み、ゲームが読むアクションを更新するモジュール。
/// </summary>
class Input : public IModule {
public:
	std::vector<std::string> GetProvidedServices() override;
	std::vector<std::string> GetRequiredServices() override;
	void Startup(ServiceRegistry& services, FrameLoop& frameLoop) override;

private:
	IWindow* window_ = nullptr;
	InputDevices devices_;
	InputActions actions_;
};

} // namespace KujataEngine
