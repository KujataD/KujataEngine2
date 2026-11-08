#pragma once

#include "Core/IModule.h"

namespace KujataEngine {

/// <summary>
/// 指定のフレーム数を実行したらAppを終了させる、動作確認用のモジュール。
/// </summary>
class QuitAfterFrames : public IModule {
public:
	QuitAfterFrames(int frameCount);

	void Startup(ServiceRegistry& services, FrameLoop& frameLoop) override;

private:
	void Update();

	int remainingFrames_;
	FrameLoop* frameLoop_ = nullptr;
};

} // namespace KujataEngine
