#pragma once

#include "IndexBuffer.h"
#include "Interface/IAssetLibrary.h"
#include "Interface/IGpu.h"
#include "ShaderResourceHeap.h"
#include "Texture.h"
#include "VertexBuffer.h"

#include <vector>

namespace KujataEngine {

/// <summary>
/// 1つのメッシュをGPUから読むためのバッファの組。
/// </summary>
struct GpuMesh {
	VertexBuffer vertexBuffer;
	IndexBuffer indexBuffer;
};

/// <summary>
/// アセットのハンドルの番号で、GPUの資源を引くための対応表。
/// </summary>
class GpuAssetTable {
public:
	/// <summary>
	/// 読み込み済みの全メッシュと全画像を、フレームの外でまとめてGPUへ転送する。
	/// </summary>
	void UploadAll(IGpu* gpu, ShaderResourceHeap* heap, IAssetLibrary* assets);

	/// <summary>
	/// メッシュを返し、転送していないハンドルなら0番を返す。
	/// </summary>
	GpuMesh& GetMesh(MeshHandle handle);

	/// <summary>
	/// テクスチャを返し、転送していないハンドルなら0番を返す。
	/// </summary>
	Texture& GetTexture(TextureHandle handle);

private:
	void CreateMeshes(ID3D12Device* device, IAssetLibrary* assets);
	void CreateTextures(ID3D12Device* device, ShaderResourceHeap* heap, IAssetLibrary* assets);
	void WarnNotUploaded();

	std::vector<GpuMesh> meshes_;
	std::vector<Texture> textures_;
	bool hasWarnedNotUploaded_ = false;
};

} // namespace KujataEngine
