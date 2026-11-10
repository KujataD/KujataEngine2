#include "Time.h"

#include <algorithm>

namespace KujataEngine {

// 止まった後に一度に大きく進まないようにする上限で、単位は秒。
static const float kMaxDeltaTime = 1.0f / 30.0f;

/// <summary>
/// 2つの時刻の差を秒で返す。
/// </summary>
static double SecondsBetween(std::chrono::steady_clock::time_point begin, std::chrono::steady_clock::time_point end) {
	return std::chrono::duration<double>(end - begin).count();
}

std::vector<std::string> Time::GetProvidedServices() {
	return { kTimeServiceName };
}

void Time::Startup(ServiceRegistry& services, FrameLoop& frameLoop) {
	startTime_ = std::chrono::steady_clock::now();
	previousTime_ = startTime_;

	// 受け取る側がITime*に戻せるように、void*にする前にITime*へ変換する。
	services.Register(kTimeServiceName, static_cast<ITime*>(this));

	// ゲームの更新より前の段階で、今のフレームの秒数を決める。
	frameLoop.Register(kBeginFrame, [this]() {
		Measure();
	});
}

float Time::GetDeltaTime() {
	return unscaledDeltaTime_ * timeScale_;
}

float Time::GetUnscaledDeltaTime() {
	return unscaledDeltaTime_;
}

void Time::SetTimeScale(float scale) {
	timeScale_ = (std::max)(scale, 0.0f);
}

float Time::GetTimeScale() {
	return timeScale_;
}

float Time::GetRealTimeSinceStartup() {
	return static_cast<float>(SecondsBetween(startTime_, std::chrono::steady_clock::now()));
}

void Time::Measure() {
	const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	const float elapsedTime = static_cast<float>(SecondsBetween(previousTime_, now));
	previousTime_ = now;

	unscaledDeltaTime_ = std::clamp(elapsedTime, 0.0f, kMaxDeltaTime);
}

} // namespace KujataEngine
