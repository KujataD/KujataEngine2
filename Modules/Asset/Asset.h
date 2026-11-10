#pragma once

#include "AssetLibrary.h"
#include "Core/IModule.h"
#include "WicTextureLoader.h"

namespace KujataEngine {

/// <summary>
/// ファイルの読み込みの準備をして、AssetLibraryを他のモジュールへ渡すモジュール。
/// </summary>
class Asset : public IModule {
public:
	std::vector<std::string> GetProvidedServices() override;
	void Startup(ServiceRegistry& services, FrameLoop& frameLoop) override;
	void Shutdown() override;

private:
	AssetLibrary library_;
	WicTextureLoader wicTextureLoader_;
	bool hasInitializedCom_ = false;
};

} // namespace KujataEngine
