#include "Core/App.h"
#include "Window/Window.h"

int main() {
	KujataEngine::App app;

	// ウィンドウを作ってAppに入れると、閉じたときにループが終わる。
	KujataEngine::Window window(L"KujataEngine2", 1280, 720);
	app.Add(&window);

	app.Run();
	return 0;
}
