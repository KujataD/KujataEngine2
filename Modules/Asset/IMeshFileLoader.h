#pragma once

#include "Interface/AssetData.h"

#include <filesystem>

namespace KujataEngine {

/// <summary>
/// モデルのファイルから読み取った内容で、画像がなければtexturePathは空になる。
/// </summary>
struct LoadedModelFile {
	MeshData mesh;
	std::filesystem::path texturePath;
};

/// <summary>
/// モデルのファイル形式ごとの読み込みを担当する窓口。
/// </summary>
class IMeshFileLoader {
public:
	virtual ~IMeshFileLoader() = default;

	/// <summary>
	/// ファイルを読み、読めなければfalseを返す。
	/// </summary>
	virtual bool Load(const std::filesystem::path& filePath, LoadedModelFile* result) = 0;
};

} // namespace KujataEngine
