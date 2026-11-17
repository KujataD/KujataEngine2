#pragma once

#include "GraphicsKit/IndexBuffer.h"
#include "GraphicsKit/ShaderResourceHeap.h"
#include "GraphicsKit/Texture.h"
#include "GraphicsKit/VertexBuffer.h"
#include "Interface/IAssetLibrary.h"
#include "Interface/IGpu.h"

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
/// 1つの点群の集まる先を、シェーダーから読むためのバッファと名札。
/// </summary>
struct GpuPointCloud {
	Microsoft::WRL::ComPtr<ID3D12Resource> targetBuffer;
	D3D12_GPU_DESCRIPTOR_HANDLE targetsGpuHandle = {};
	UINT targetCount = 0;
};

/// <summary>
/// アセットのハンドルの番号で、GPUの資源を引くための対応表。
/// </summary>
class GpuAssetTable {
public:
	/// <summary>
	/// 読み込み済みの全メッシュと全画像と全点群を、フレームの外でまとめてGPUへ転送する。
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

	/// <summary>
	/// 点群の集まる先を返し、転送していないハンドルなら0番を返す。
	/// </summary>
	GpuPointCloud& GetPointCloud(PointCloudHandle handle);

private:
	void CreateMeshes(ID3D12Device* device, IAssetLibrary* assets);
	void CreateTextures(ID3D12Device* device, ShaderResourceHeap* heap, IAssetLibrary* assets);
	void CreatePointClouds(IGpu* gpu, ShaderResourceHeap* heap, IAssetLibrary* assets);
	void WarnNotUploaded();

	std::vector<GpuMesh> meshes_;
	std::vector<Texture> textures_;
	std::vector<GpuPointCloud> pointClouds_;
	bool hasWarnedNotUploaded_ = false;
};

} // namespace KujataEngine
