#pragma once

#include <functional>
#include <string>

namespace KujataEngine {

// ServiceRegistryに登録するときの名前。
const char* const kConsoleCommandsServiceName = "ConsoleCommands";

/// <summary>
/// ターミナルで打った名前で処理を呼べるように、コマンドを登録するための窓口。
/// </summary>
class IConsoleCommands {
public:
	virtual ~IConsoleCommands() = default;

	/// <summary>
	/// ターミナルで指定の名前が打たれたら、次のフレームの頭で渡された処理を呼ぶ。
	/// </summary>
	virtual void AddCommand(const std::string& name, std::function<void()> action) = 0;
};

} // namespace KujataEngine
