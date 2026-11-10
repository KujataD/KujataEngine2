#pragma once

#include "Interface/AssetData.h"
#include "Interface/AssetHandle.h"

#include <cstdint>
#include <string>

namespace KujataEngine {

// ServiceRegistryに登録するときの名前。
const char* const kAssetLibraryServiceName = "AssetLibrary";

/// <summary>
/// ファイルから読んだデータをハンドルで受け渡すための窓口。
/// </summary>
class IAssetLibrary {
public:
	virtual ~IAssetLibrary() = default;

	/// <summary>
	/// データのフォルダからの相対パスでモデルを読み、読めなければ代わりの物を返す。
	/// </summary>
	virtual ModelHandles LoadModel(const std::string& utf8Path) = 0;

	/// <summary>
	/// データのフォルダからの相対パスで画像を読み、読めなければ代わりの物を返す。
	/// </summary>
	virtual TextureHandle LoadTexture(const std::string& utf8Path) = 0;

	/// <summary>
	/// 立方体のメッシュを返す。
	/// </summary>
	virtual MeshHandle GetCubeMesh() = 0;

	/// <summary>
	/// コードで作ったメッシュを登録し、不正なデータなら代わりの物を返す。
	/// </summary>
	virtual MeshHandle AddMesh(const MeshData& mesh) = 0;

	// Rendererが転送のために読む。
	virtual uint32_t GetMeshCount() = 0;
	virtual const MeshData& GetMeshData(MeshHandle handle) = 0;
	virtual uint32_t GetTextureCount() = 0;
	virtual const ImageData& GetImageData(TextureHandle handle) = 0;
};

} // namespace KujataEngine
