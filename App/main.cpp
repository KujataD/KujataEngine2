#include "Asset/Asset.h"
#include "Console/Console.h"
#include "Core/ModuleHub.h"
#include "DemoScene.h"
#include "GameScene.h"
#include "Gpu/Gpu.h"
#include "ImGuiModule/ImGuiModule.h"
#include "Input/Input.h"
#include "LineRendering/LineRendering.h"
#include "ParticleRendering/ParticleRendering.h"
#include "Physics/Physics.h"
#include "Renderer/Renderer.h"
#include "Scene/SceneModule.h"
#include "Time/Time.h"
#include "Window/Window.h"

#include <Windows.h>

// trueでゲームの代わりにデモのシーンを組む。
static constexpr bool kBuildDemoScene = false;

int main() {
	// コンソールをUTF-8にそろえる。
	SetConsoleOutputCP(CP_UTF8);

	KujataEngine::ModuleHub hub;

	// ほかのモジュールより先に、今のフレームの秒数を測る。
	KujataEngine::Time timeModule;
	hub.Add(&timeModule);

	// ウィンドウを作ってModuleHubに入れると、閉じたときにループが終わる。
	KujataEngine::Window window(L"LE2B_03_オオツカ_ダイチ", 1280, 720);
	hub.Add(&window);

	KujataEngine::Input input;
	hub.Add(&input);

	KujataEngine::Gpu gpu;
	hub.Add(&gpu);

	KujataEngine::Asset asset;
	hub.Add(&asset);

	KujataEngine::Renderer renderer;
	hub.Add(&renderer);

	// 線を描く工程を、Rendererの登録口に足す。
	KujataEngine::LineRendering lineRendering;
	hub.Add(&lineRendering);

	// 粒子を動かす工程と描く工程を、Rendererの登録口に足す。
	KujataEngine::ParticleRendering particleRendering;
	hub.Add(&particleRendering);

	// シーンやほかのモジュールが、フレームの中でimguiのウィンドウを組めるようにする。
	KujataEngine::ImGuiModule imguiModule;
	hub.Add(&imguiModule);

	KujataEngine::SceneModule sceneModule;
	hub.Add(&sceneModule);

	KujataEngine::Physics physics;
	hub.Add(&physics);

	// ターミナルに打ったコマンドを実行できるようにする。
	KujataEngine::Console console;
	hub.Add(&console);

	// Assetなどの起動が終わってからシーンを作る。
	hub.SetSetup([&sceneModule](KujataEngine::ServiceRegistry& services) {
		KujataEngine::IAssetLibrary* assets = static_cast<KujataEngine::IAssetLibrary*>(services.Get(KujataEngine::kAssetLibraryServiceName));
		KujataEngine::IInputActions* inputActions = static_cast<KujataEngine::IInputActions*>(services.Get(KujataEngine::kInputActionsServiceName));
		KujataEngine::ITime* time = static_cast<KujataEngine::ITime*>(services.Get(KujataEngine::kTimeServiceName));

		if (kBuildDemoScene) {
			KujataEngine::IConsoleCommands* consoleCommands = static_cast<KujataEngine::IConsoleCommands*>(services.Get(KujataEngine::kConsoleCommandsServiceName));
			KujataEngine::ILineRenderer* lineRenderer = static_cast<KujataEngine::ILineRenderer*>(services.Get(KujataEngine::kLineRendererServiceName));
			KujataEngine::IParticleRenderer* particleRenderer = static_cast<KujataEngine::IParticleRenderer*>(services.Get(KujataEngine::kParticleRendererServiceName));
			BuildDemoScene(sceneModule.GetScene(), assets, inputActions, time, consoleCommands, lineRenderer, particleRenderer);
			return;
		}

		KujataEngine::IWindow* window = static_cast<KujataEngine::IWindow*>(services.Get(KujataEngine::kWindowServiceName));
		BuildGameScene(sceneModule.GetScene(), assets, inputActions, time, window);
	});

	hub.Run();
	return 0;
}
