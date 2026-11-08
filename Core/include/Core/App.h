#pragma once

#include "Core/FrameLoop.h"
#include "Core/IModule.h"
#include "Core/ServiceRegistry.h"

#include <vector>

namespace KujataEngine {

/// <summary>
/// モジュールを集めて、起動とフレームの実行と終了を順番に進める。
/// </summary>
class App {
public:
	/// <summary>
	/// モジュールを一覧に入れるだけで、まだ動かさない。
	/// </summary>
	void Add(IModule* addedModule);

	/// <summary>
	/// 追加した順に起動し、ループを抜けたら逆の順に終了する。
	/// </summary>
	void Run();

private:
	std::vector<IModule*> modules_;
	ServiceRegistry services_;
	FrameLoop frameLoop_;
};

} // namespace KujataEngine
