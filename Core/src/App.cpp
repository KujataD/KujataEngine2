#include "Core/App.h"

namespace KujataEngine {

void App::Add(IModule* addedModule) {
	modules_.push_back(addedModule);
}

void App::Run() {
	for (IModule* addedModule : modules_) {
		addedModule->Startup(services_, frameLoop_);
	}

	frameLoop_.Run();

	// 後から起動したモジュールほど他に頼っているので、先に終了する。
	for (int i = static_cast<int>(modules_.size()) - 1; i >= 0; i--) {
		modules_[i]->Shutdown();
	}
}

} // namespace KujataEngine
