#pragma once

#include "Core/IModule.h"
#include "Interface/IConsoleCommands.h"

#include <map>
#include <memory>
#include <mutex>
#include <vector>

namespace KujataEngine {

/// <summary>
/// ターミナルを読むスレッドとゲームのスレッドの間で、打たれた行を受け渡す置き場。
/// </summary>
struct ConsoleInputQueue {
	// 2つのスレッドが同時にlinesを触らないようにする鍵。
	std::mutex mutex;
	std::vector<std::string> lines;
};

/// <summary>
/// ターミナルに打たれた行を読み、同じ名前で登録されたコマンドをフレームの頭で実行するモジュール。
/// </summary>
class Console : public IModule, public IConsoleCommands {
public:
	std::vector<std::string> GetProvidedServices() override;
	void Startup(ServiceRegistry& services, FrameLoop& frameLoop) override;
	void Shutdown() override;

	void AddCommand(const std::string& name, std::function<void()> action) override;

private:
	/// <summary>
	/// 前のフレームから今までに打たれた行を取り出し、登録されたコマンドを実行する。
	/// </summary>
	void RunPendingCommands();

	/// <summary>
	/// 登録されているコマンドの名前をログに並べる。
	/// </summary>
	void PrintCommandNames();

	std::map<std::string, std::function<void()>> commands_;

	// 読むスレッドはモジュールより長く動くことがあるので、置き場は両方から共有して持つ。
	std::shared_ptr<ConsoleInputQueue> inputQueue_;
};

} // namespace KujataEngine
