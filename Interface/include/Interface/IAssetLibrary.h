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
	/// 画像の指定がない物に使う、白い1x1の画像を返す。
	/// </summary>
	virtual TextureHandle GetWhiteTexture() = 0;

	/// <summary>
	/// コードで作ったメッシュを登録し、不正なデータなら代わりの物を返す。
	/// </summary>
	virtual MeshHandle AddMesh(const MeshData& mesh) = 0;

	// Rendererが転送のために読む。

	/// <summary>
	/// 登録されているメッシュの数を返す。
	/// </summary>
	virtual uint32_t GetMeshCount() = 0;

	/// <summary>
	/// メッシュのデータを返し、範囲外のハンドルなら0番を返す。
	/// </summary>
	virtual const MeshData& GetMeshData(MeshHandle handle) = 0;

	/// <summary>
	/// 登録されている画像の数を返す。
	/// </summary>
	virtual uint32_t GetTextureCount() = 0;

	/// <summary>
	/// 画像のデータを返し、範囲外のハンドルなら0番を返す。
	/// </summary>
	virtual const ImageData& GetImageData(TextureHandle handle) = 0;
};

} // namespace KujataEngine
