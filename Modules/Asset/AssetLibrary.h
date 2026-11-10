#pragma once

#include "IMeshFileLoader.h"
#include "ITextureFileLoader.h"
#include "Interface/IAssetLibrary.h"

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace KujataEngine {

/// <summary>
/// 読み込んだデータをハンドルの番号で持ち、同じファイルは1回だけ読むクラス。
/// </summary>
class AssetLibrary : public IAssetLibrary {
public:
	/// <summary>
	/// データのフォルダを覚えて、0番の代わりの物と1番の白い画像を作る。
	/// </summary>
	void Initialize(const std::filesystem::path& dataDirectory);

	/// <summary>
	/// 小文字の拡張子に対応するモデルの読み込みを登録する。
	/// </summary>
	void AddMeshLoader(const std::wstring& extension, IMeshFileLoader* loader);

	/// <summary>
	/// 小文字の拡張子に対応する画像の読み込みを登録する。
	/// </summary>
	void AddTextureLoader(const std::wstring& extension, ITextureFileLoader* loader);

	ModelHandles LoadModel(const std::string& utf8Path) override;
	TextureHandle LoadTexture(const std::string& utf8Path) override;
	MeshHandle GetCubeMesh() override;
	TextureHandle GetWhiteTexture() override;
	MeshHandle AddMesh(const MeshData& mesh) override;

	uint32_t GetMeshCount() override;
	const MeshData& GetMeshData(MeshHandle handle) override;
	uint32_t GetTextureCount() override;
	const ImageData& GetImageData(TextureHandle handle) override;

private:
	ModelHandles ReadModelFile(const std::filesystem::path& filePath);
	TextureHandle LoadTextureFile(const std::filesystem::path& filePath);
	TextureHandle ReadTextureFile(const std::filesystem::path& filePath);

	std::filesystem::path dataDirectory_;

	// ハンドルの番号がそのまま配列の番号になる。
	std::vector<MeshData> meshes_;
	std::vector<ImageData> images_;

	std::map<std::wstring, IMeshFileLoader*> meshLoaders_;
	std::map<std::wstring, ITextureFileLoader*> textureLoaders_;

	// 失敗も覚えるので、同じファイルのログは1回だけ出る。
	std::map<std::filesystem::path, ModelHandles> loadedModels_;
	std::map<std::filesystem::path, TextureHandle> loadedTextures_;
};

} // namespace KujataEngine
