#pragma once

#include <filesystem>
#include <string>

namespace KujataEngine {

/// <summary>
/// UTF-8の文字列をパスに変換する。
/// </summary>
std::filesystem::path MakePathFromUtf8(const std::string& utf8Text);

/// <summary>
/// パスをログに出せるUTF-8の文字列に変換する。
/// </summary>
std::string MakeUtf8FromPath(const std::filesystem::path& filePath);

} // namespace KujataEngine
