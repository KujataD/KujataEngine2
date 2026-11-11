#include "Asset/Asset.h"
#include "Core/App.h"
#include "DemoScene.h"
#include "Gpu/Gpu.h"
#include "Input/Input.h"
#include "Physics/Physics.h"
#include "Renderer/Renderer.h"
#include "Scene/SceneModule.h"
#include "Time/Time.h"
#include "Window/Window.h"

#include <Windows.h>

int main() {
	// コンソールをUTF-8にそろえる。
	SetConsoleOutputCP(CP_UTF8);

	KujataEngine::App app;

	// ほかのモジュールより先に、今のフレームの秒数を測る。
	KujataEngine::Time timeModule;
	app.Add(&timeModule);

	// ウィンドウを作ってAppに入れると、閉じたときにループが終わる。
	KujataEngine::Window window(L"KujataEngine2", 1280, 720);
	app.Add(&window);

	KujataEngine::Input input;
	app.Add(&input);

	KujataEngine::Gpu gpu;
	app.Add(&gpu);

	KujataEngine::Asset asset;
	app.Add(&asset);

	KujataEngine::Renderer renderer;
	app.Add(&renderer);

	KujataEngine::SceneModule sceneModule;
	app.Add(&sceneModule);

	KujataEngine::Physics physics;
	app.Add(&physics);

	// Assetなどの起動が終わってからシーンを作る。
	app.SetSetup([&sceneModule](KujataEngine::ServiceRegistry& services) {
		KujataEngine::IAssetLibrary* assets = static_cast<KujataEngine::IAssetLibrary*>(services.Get(KujataEngine::kAssetLibraryServiceName));
		KujataEngine::IInputActions* inputActions = static_cast<KujataEngine::IInputActions*>(services.Get(KujataEngine::kInputActionsServiceName));
		KujataEngine::ITime* time = static_cast<KujataEngine::ITime*>(services.Get(KujataEngine::kTimeServiceName));
		KujataEngine::IWindow* window = static_cast<KujataEngine::IWindow*>(services.Get(KujataEngine::kWindowServiceName));
		BuildDemoScene(sceneModule.GetScene(), assets, inputActions, time, window);
	});

	app.Run();
	return 0;
}
