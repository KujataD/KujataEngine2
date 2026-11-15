#include "Console.h"

#include <cstdio>
#include <iostream>
#include <thread>

namespace KujataEngine {

/// <summary>
/// 行の前後の空白と改行の文字を取り除く。
/// </summary>
static std::string TrimSpaces(const std::string& text) {
	const char* const spaces = " \t\r\n";
	const size_t begin = text.find_first_not_of(spaces);
	if (begin == std::string::npos) {
		return "";
	}
	const size_t end = text.find_last_not_of(spaces);
	return text.substr(begin, end - begin + 1);
}

/// <summary>
/// PowerShellからパイプで渡したときに最初の行の頭に付く、UTF-8の目印の3バイトを取り除く。
/// </summary>
static std::string RemoveByteOrderMark(const std::string& text) {
	const std::string byteOrderMark = "\xEF\xBB\xBF";
	if (text.compare(0, byteOrderMark.size(), byteOrderMark) == 0) {
		return text.substr(byteOrderMark.size());
	}
	return text;
}

/// <summary>
/// ターミナルから1行ずつ読んで置き場に積み続け、入力が閉じられたら終わる。
/// </summary>
static void ReadLinesIntoQueue(std::shared_ptr<ConsoleInputQueue> inputQueue) {
	std::string line;
	while (std::getline(std::cin, line)) {
		std::lock_guard<std::mutex> lock(inputQueue->mutex);
		inputQueue->lines.push_back(line);
	}
}

std::vector<std::string> Console::GetProvidedServices() {
	return { kConsoleCommandsServiceName };
}

void Console::Startup(ServiceRegistry& services, FrameLoop& frameLoop) {
	inputQueue_ = std::make_shared<ConsoleInputQueue>();

	// 1行読むのはEnterが押されるまで戻らないので、フレームを止めないように別のスレッドで待つ。
	std::thread readerThread(ReadLinesIntoQueue, inputQueue_);

	// 終了のときも読み取りが戻るのを待てないので、切り離して待たずに終われるようにする。
	readerThread.detach();

	// 受け取る側がIConsoleCommands*に戻せるように、void*にする前にIConsoleCommands*へ変換する。
	services.Register(kConsoleCommandsServiceName, static_cast<IConsoleCommands*>(this));

	// コマンドの処理がゲームの更新の途中に割り込まないように、フレームの頭で実行する。
	frameLoop.Register(kBeginFrame, [this]() {
		RunPendingCommands();
	});
}

void Console::Shutdown() {
	// 読むスレッドが入力を待ったままだと、終了のときにログの残りが出ないことがあるので、ここで出し切る。
	fflush(stdout);
}

void Console::AddCommand(const std::string& name, std::function<void()> action) {
	commands_[name] = action;
}

void Console::RunPendingCommands() {
	// 鍵を持つ時間を短くするために、積まれた行を手元に移してから実行する。
	std::vector<std::string> lines;
	{
		std::lock_guard<std::mutex> lock(inputQueue_->mutex);
		lines.swap(inputQueue_->lines);
	}

	for (const std::string& line : lines) {
		const std::string name = TrimSpaces(RemoveByteOrderMark(line));
		if (name.empty()) {
			continue;
		}

		if (commands_.count(name) == 0) {
			printf("知らないコマンドです: %s\n", name.c_str());
			PrintCommandNames();
			continue;
		}
		commands_[name]();
	}
}

void Console::PrintCommandNames() {
	printf("使えるコマンド:");
	for (const std::pair<const std::string, std::function<void()>>& command : commands_) {
		printf(" %s", command.first.c_str());
	}
	printf("\n");
}

} // namespace KujataEngine
