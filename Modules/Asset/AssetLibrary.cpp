#include "AssetLibrary.h"

#include "AssetPath.h"
#include "CubeMesh.h"

#include <cstdio>
#include <cwctype>
#include <utility>

namespace KujataEngine {

/// <summary>
/// ファイルの拡張子を小文字にして返す。
/// </summary>
static std::wstring GetLowerExtension(const std::filesystem::path& filePath) {
	std::wstring extension = filePath.extension().wstring();
	for (wchar_t& character : extension) {
		character = static_cast<wchar_t>(std::towlower(character));
	}
	return extension;
}

/// <summary>
/// 読み込みに失敗したときに使う1x1のピンクの画像を作る。
/// </summary>
static ImageData CreatePinkImage() {
	ImageMip mip;
	mip.width = 1;
	mip.height = 1;
	mip.rowPitch = 4;
	mip.pixels = { 255, 0, 255, 255 };

	ImageData image;
	image.mips.push_back(mip);
	return image;
}

void AssetLibrary::Initialize(const std::filesystem::path& dataDirectory) {
	dataDirectory_ = dataDirectory;

	// 0番は失敗したときの代わりなので、最初に作る。
	meshes_.push_back(CreateCubeMesh());
	images_.push_back(CreatePinkImage());
}

void AssetLibrary::AddMeshLoader(const std::wstring& extension, IMeshFileLoader* loader) {
	meshLoaders_[extension] = loader;
}

void AssetLibrary::AddTextureLoader(const std::wstring& extension, ITextureFileLoader* loader) {
	textureLoaders_[extension] = loader;
}

ModelHandles AssetLibrary::LoadModel(const std::string& utf8Path) {
	const std::filesystem::path filePath = (dataDirectory_ / MakePathFromUtf8(utf8Path)).lexically_normal();
	if (loadedModels_.count(filePath) > 0) {
		return loadedModels_[filePath];
	}

	const ModelHandles handles = ReadModelFile(filePath);
	loadedModels_[filePath] = handles;
	return handles;
}

TextureHandle AssetLibrary::LoadTexture(const std::string& utf8Path) {
	return LoadTextureFile((dataDirectory_ / MakePathFromUtf8(utf8Path)).lexically_normal());
}

MeshHandle AssetLibrary::GetCubeMesh() {
	return MeshHandle();
}

MeshHandle AssetLibrary::AddMesh(const MeshData& mesh) {
	if (mesh.vertices.empty() || mesh.indices.empty()) {
		printf("空のメッシュは登録できないので、代わりの立方体を使います。\n");
		return MeshHandle();
	}

	for (uint32_t index : mesh.indices) {
		if (index >= mesh.vertices.size()) {
			printf("頂点の番号が範囲外なので、代わりの立方体を使います。\n");
			return MeshHandle();
		}
	}

	MeshHandle handle;
	handle.id = static_cast<uint32_t>(meshes_.size());
	meshes_.push_back(mesh);
	return handle;
}

uint32_t AssetLibrary::GetMeshCount() {
	return static_cast<uint32_t>(meshes_.size());
}

const MeshData& AssetLibrary::GetMeshData(MeshHandle handle) {
	if (handle.id >= meshes_.size()) {
		return meshes_[0];
	}
	return meshes_[handle.id];
}

uint32_t AssetLibrary::GetTextureCount() {
	return static_cast<uint32_t>(images_.size());
}

const ImageData& AssetLibrary::GetImageData(TextureHandle handle) {
	if (handle.id >= images_.size()) {
		return images_[0];
	}
	return images_[handle.id];
}

ModelHandles AssetLibrary::ReadModelFile(const std::filesystem::path& filePath) {
	const std::wstring extension = GetLowerExtension(filePath);
	if (meshLoaders_.count(extension) == 0) {
		printf("対応していないモデルの形式なので、代わりの物を使います: %s\n", MakeUtf8FromPath(filePath).c_str());
		return ModelHandles();
	}

	LoadedModelFile loaded;
	if (!meshLoaders_[extension]->Load(filePath, &loaded)) {
		printf("モデルを読めなかったので、代わりの物を使います: %s\n", MakeUtf8FromPath(filePath).c_str());
		return ModelHandles();
	}

	ModelHandles handles;
	handles.mesh = AddMesh(loaded.mesh);
	if (!loaded.texturePath.empty()) {
		handles.texture = LoadTextureFile(loaded.texturePath);
	}
	return handles;
}

TextureHandle AssetLibrary::LoadTextureFile(const std::filesystem::path& filePath) {
	if (loadedTextures_.count(filePath) > 0) {
		return loadedTextures_[filePath];
	}

	const TextureHandle handle = ReadTextureFile(filePath);
	loadedTextures_[filePath] = handle;
	return handle;
}

TextureHandle AssetLibrary::ReadTextureFile(const std::filesystem::path& filePath) {
	const std::wstring extension = GetLowerExtension(filePath);
	if (textureLoaders_.count(extension) == 0) {
		printf("対応していない画像の形式なので、代わりの物を使います: %s\n", MakeUtf8FromPath(filePath).c_str());
		return TextureHandle();
	}

	ImageData image;
	if (!textureLoaders_[extension]->Load(filePath, &image)) {
		printf("画像を読めなかったので、代わりの物を使います: %s\n", MakeUtf8FromPath(filePath).c_str());
		return TextureHandle();
	}

	TextureHandle handle;
	handle.id = static_cast<uint32_t>(images_.size());
	images_.push_back(std::move(image));
	return handle;
}

} // namespace KujataEngine
