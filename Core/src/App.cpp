#include "Core/App.h"

#include "Core/Assert.h"

#include <cstdio>
#include <set>

namespace KujataEngine {

/// <summary>
/// 必要なサービスがすべてそろっていればtrueを返す。
/// </summary>
static bool HasAllRequiredServices(IModule* module, const std::set<std::string>& availableServices) {
	for (const std::string& requiredService : module->GetRequiredServices()) {
		if (availableServices.count(requiredService) == 0) {
			return false;
		}
	}
	return true;
}

/// <summary>
/// 起動できないモジュールが待っているサービスの名前をコンソールに出す。
/// </summary>
static void PrintMissingServices(const std::vector<IModule*>& waitingModules, const std::set<std::string>& availableServices) {
	for (IModule* waitingModule : waitingModules) {
		for (const std::string& requiredService : waitingModule->GetRequiredServices()) {
			if (availableServices.count(requiredService) == 0) {
				printf("サービスが足りないか循環しています: %s\n", requiredService.c_str());
			}
		}
	}
}

void App::Add(IModule* addedModule) {
	KUJATA_ASSERT(addedModule != nullptr);

	modules_.push_back(addedModule);
}

void App::SortModulesByDependency() {
	std::vector<IModule*> sortedModules;
	std::vector<IModule*> waitingModules = modules_;
	std::set<std::string> availableServices;

	while (!waitingModules.empty()) {
		// 追加した順に見るので、依存がないもの同士の順番は毎回同じになる。
		int readyIndex = -1;
		for (int i = 0; i < static_cast<int>(waitingModules.size()); i++) {
			if (HasAllRequiredServices(waitingModules[i], availableServices)) {
				readyIndex = i;
				break;
			}
		}

		if (readyIndex < 0) {
			PrintMissingServices(waitingModules, availableServices);
		}
		KUJATA_ASSERT(readyIndex >= 0);

		IModule* readyModule = waitingModules[readyIndex];
		sortedModules.push_back(readyModule);
		waitingModules.erase(waitingModules.begin() + readyIndex);

		for (const std::string& providedService : readyModule->GetProvidedServices()) {
			availableServices.insert(providedService);
		}
	}

	modules_ = sortedModules;
}

void App::Run() {
	SortModulesByDependency();

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
