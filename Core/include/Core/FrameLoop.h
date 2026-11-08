#pragma once

#include <functional>
#include <vector>

namespace KujataEngine {

/// <summary>
/// 1フレームの中の段階で、上から順に実行される。
/// </summary>
enum FramePhase {
	kBeginFrame,
	kUpdate,
	kSubmitRender,
	kRender,
	kEndFrame,
	kFramePhaseCount,
};

/// <summary>
/// 段階ごとに登録された処理を、終了を頼まれるまで毎フレーム実行する。
/// </summary>
class FrameLoop {
public:
	/// <summary>
	/// 指定した段階で毎フレーム実行する処理を登録する。
	/// </summary>
	void Register(FramePhase phase, std::function<void()> callback);

	/// <summary>
	/// 今のフレームを最後まで実行してからループを抜ける。
	/// </summary>
	void RequestQuit();

	/// <summary>
	/// 処理がひとつも登録されていなければ、すぐに戻る。
	/// </summary>
	void Run();

private:
	bool HasCallback();
	void RunFrame();

	// 段階の数だけ、処理の一覧を持つ。
	std::vector<std::function<void()>> callbacks_[kFramePhaseCount];
	bool isQuitRequested_ = false;
};

} // namespace KujataEngine
