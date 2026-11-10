#include "Asset.h"

#include <Windows.h>
#include <cstdio>
#include <filesystem>

namespace KujataEngine {

/// <summary>
/// exeの場所から、ゲームのデータがあるフォルダを求める。
/// </summary>
static std::filesystem::path GetAssetDataDirectory() {
	wchar_t exePath[MAX_PATH] = {};
	GetModuleFileNameW(nullptr, exePath, MAX_PATH);

	const std::filesystem::path exeDirectory = std::filesystem::path(exePath).parent_path();

	// 配布したexeは、隣のDataを使う。
	const std::filesystem::path packagedDirectory = exeDirectory / L"Data" / L"Game";
	if (std::filesystem::exists(packagedDirectory)) {
		return packagedDirectory;
	}

	// 開発中のexeはbuild\bin\構成名にあるので、3つ上がリポジトリの一番上になる。
	const std::filesystem::path repositoryRoot = exeDirectory.parent_path().parent_path().parent_path();
	return repositoryRoot / L"App" / L"Data";
}

std::vector<std::string> Asset::GetProvidedServices() {
	return { kAssetLibraryServiceName };
}

void Asset::Startup(ServiceRegistry& services, FrameLoop& frameLoop) {
	// 画像の読み込みにCOMを使う。
	hasInitializedCom_ = SUCCEEDED(CoInitializeEx(nullptr, COINIT_MULTITHREADED));
	if (!hasInitializedCom_) {
		printf("COMを初期化できなかったので、画像を読めないことがあります。\n");
	}

	library_.Initialize(GetAssetDataDirectory());
	library_.AddMeshLoader(L".obj", &objMeshLoader_);
	library_.AddTextureLoader(L".png", &wicTextureLoader_);
	library_.AddTextureLoader(L".jpg", &wicTextureLoader_);
	library_.AddTextureLoader(L".jpeg", &wicTextureLoader_);

	// 受け取る側がIAssetLibrary*に戻せるように、void*にする前にIAssetLibrary*へ変換する。
	services.Register(kAssetLibraryServiceName, static_cast<IAssetLibrary*>(&library_));
}

void Asset::Shutdown() {
	if (hasInitializedCom_) {
		CoUninitialize();
	}
}

} // namespace KujataEngine
