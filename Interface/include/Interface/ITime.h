#pragma once

namespace KujataEngine {

// ServiceRegistryに登録するときの名前。
const char* const kTimeServiceName = "Time";

/// <summary>
/// ゲームが前のフレームからの時間を読むための窓口。
/// </summary>
class ITime {
public:
	virtual ~ITime() = default;

	/// <summary>
	/// 前のフレームからの秒数に時間の速さを掛けた値で、ゲームの動きはこれを使う。
	/// </summary>
	virtual float GetDeltaTime() = 0;

	/// <summary>
	/// 時間の速さを掛けない前のフレームからの秒数で、ヒットストップの制御やUIに使う。
	/// </summary>
	virtual float GetUnscaledDeltaTime() = 0;

	/// <summary>
	/// 時間の進む速さを1で等速、0で停止として決め、戻すのは決めた側が行う。
	/// </summary>
	virtual void SetTimeScale(float scale) = 0;

	/// <summary>
	/// 時間の進む速さを返す。
	/// </summary>
	virtual float GetTimeScale() = 0;

	/// <summary>
	/// 起動してからの実時間の秒数で、速さに関係なく進むので見た目だけに使う。
	/// </summary>
	virtual float GetRealTimeSinceStartup() = 0;
};

} // namespace KujataEngine
