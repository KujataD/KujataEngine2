#pragma once

#include "Core/IModule.h"
#include "Interface/ITime.h"

#include <chrono>

namespace KujataEngine {

/// <summary>
/// 前のフレームからの秒数を毎フレーム測り、ITimeとして他のモジュールへ渡すモジュール。
/// </summary>
class Time : public IModule, public ITime {
public:
	std::vector<std::string> GetProvidedServices() override;
	void Startup(ServiceRegistry& services, FrameLoop& frameLoop) override;

	float GetDeltaTime() override;
	float GetUnscaledDeltaTime() override;
	void SetTimeScale(float scale) override;
	float GetTimeScale() override;
	float GetRealTimeSinceStartup() override;

private:
	/// <summary>
	/// 前のフレームからの秒数を測り、上限で頭打ちにして覚える。
	/// </summary>
	void Measure();

	std::chrono::steady_clock::time_point startTime_;
	std::chrono::steady_clock::time_point previousTime_;
	float unscaledDeltaTime_ = 0.0f;
	float timeScale_ = 1.0f;
};

} // namespace KujataEngine
