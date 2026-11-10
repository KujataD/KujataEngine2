#include "AssetPath.h"

namespace KujataEngine {

std::filesystem::path MakePathFromUtf8(const std::string& utf8Text) {
	// そのまま渡すとWindowsの既定の文字コードとして読まれるので、UTF-8の型にしてから渡す。
	const std::u8string utf8(utf8Text.begin(), utf8Text.end());
	return std::filesystem::path(utf8);
}

std::string MakeUtf8FromPath(const std::filesystem::path& filePath) {
	const std::u8string utf8 = filePath.u8string();
	return std::string(utf8.begin(), utf8.end());
}

} // namespace KujataEngine
