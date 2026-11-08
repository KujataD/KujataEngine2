#include "Core/App.h"
#include "QuitAfterFrames/QuitAfterFrames.h"

int main() {
	App app;

	// 3フレーム動いたら終了するモジュールを作って、Appに入れる。
	QuitAfterFrames quitAfterFrames(3);
	app.Add(&quitAfterFrames);

	app.Run();
	return 0;
}
