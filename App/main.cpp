#include "Core/App.h"
#include "DemoScene.h"
#include "Gpu/Gpu.h"
#include "Physics/Physics.h"
#include "Renderer/Renderer.h"
#include "Scene/SceneModule.h"
#include "Window/Window.h"

#include <Windows.h>

int main() {
	// コンソールをUTF-8にそろえる。
	SetConsoleOutputCP(CP_UTF8);

	KujataEngine::App app;

	// ウィンドウを作ってAppに入れると、閉じたときにループが終わる。
	KujataEngine::Window window(L"KujataEngine2", 1280, 720);
	app.Add(&window);

	KujataEngine::Gpu gpu;
	app.Add(&gpu);

	KujataEngine::Renderer renderer;
	app.Add(&renderer);

	KujataEngine::SceneModule sceneModule;
	app.Add(&sceneModule);

	KujataEngine::Physics physics;
	app.Add(&physics);

	// Assetなどの起動が終わってからシーンを作る。
	app.SetSetup([&sceneModule](KujataEngine::ServiceRegistry& services) {
		BuildDemoScene(sceneModule.GetScene());
	});

	app.Run();
	return 0;
}
