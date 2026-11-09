#include "Core/App.h"
#include "DemoScene.h"
#include "Framework/World.h"
#include "Gpu/Gpu.h"
#include "Physics/Physics.h"
#include "Renderer/Renderer.h"
#include "Scene/SceneModule.h"
#include "Window/Window.h"
#include <cstdio>

#include <Windows.h>

struct TestPosition {
	float x;
};

struct TestSpeed {
	float value;
};

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
	BuildDemoScene(sceneModule.GetScene());

	KujataEngine::Physics physics;
	app.Add(&physics);

	KujataEngine::World world;

	KujataEngine::Entity player = world.CreateEntity();
	KujataEngine::Entity wall = world.CreateEntity();
	KujataEngine::Entity enemy = world.CreateEntity();

	world.Add(player, TestPosition{ 1.0f });
	world.Add(player, TestSpeed{ 0.5f });
	world.Add(wall, TestPosition{ 9.0f });
	world.Add(enemy, TestPosition{ 3.0f });
	world.Add(enemy, TestSpeed{ 2.0f });

	int callCount = 0;
	world.Each<TestPosition, TestSpeed>([&](TestPosition& position, const TestSpeed& speed) {
		position.x += speed.value;
		callCount++;
	});

	printf("callCount=%d\n", callCount);
	printf("player x=%.1f\n", world.Get<TestSpeed>(player).value);
	printf("wall x=%.1f\n", world.Get<TestPosition>(wall).x);
	printf("enemy x=%.1f\n", world.Get<TestPosition>(enemy).x);

	app.Run();
	return 0;
}
