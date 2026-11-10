#include "Input.h"

#include "Core/Assert.h"

namespace KujataEngine {

std::vector<std::string> Input::GetProvidedServices() {
	return { kInputActionsServiceName };
}

std::vector<std::string> Input::GetRequiredServices() {
	return { kWindowServiceName };
}

void Input::Startup(ServiceRegistry& services, FrameLoop& frameLoop) {
	window_ = static_cast<IWindow*>(services.Get(kWindowServiceName));
	KUJATA_ASSERT(window_ != nullptr);

	// 受け取る側がIInputActions*に戻せるように、void*にする前にIInputActions*へ変換する。
	services.Register(kInputActionsServiceName, static_cast<IInputActions*>(&actions_));

	// Windowより後に起動するので、同じ段階でもメッセージを処理した後の状態を読める。
	frameLoop.Register(kBeginFrame, [this]() {
		actions_.Update(devices_.Read(window_->GetNativeHandle()));
	});
}

} // namespace KujataEngine
