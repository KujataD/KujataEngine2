#include "Core/App.h"

#include "Core/Assert.h"

namespace KujataEngine {

void App::Add(IModule* addedModule) {
	KUJATA_ASSERT(addedModule != nullptr);

	modules_.push_back(addedModule);
}

void App::Run() {
	// 全モジュールの初期化
	for (IModule* addedModule : modules_) {
		addedModule->Startup(services_, frameLoop_);
	}

	// メインループ
	frameLoop_.Run();

	// 後から起動したモジュールほど他に頼っているので、先に終了する。
	for (int i = static_cast<int>(modules_.size()) - 1; i >= 0; i--) {
		modules_[i]->Shutdown();
	}
}

} // namespace KujataEngine
