#pragma once

#include "Interface/AssetData.h"

#include <filesystem>

namespace KujataEngine {

/// <summary>
/// 画像のファイル形式ごとの読み込みを担当する窓口。
/// </summary>
class ITextureFileLoader {
public:
	virtual ~ITextureFileLoader() = default;

	/// <summary>
	/// ファイルを読み、読めなければfalseを返す。
	/// </summary>
	virtual bool Load(const std::filesystem::path& filePath, ImageData* result) = 0;
};

} // namespace KujataEngine
