#include "GpuAssetTable.h"

#include <cstdio>

namespace KujataEngine {

void GpuAssetTable::UploadAll(IGpu* gpu, ShaderResourceHeap* heap, IAssetLibrary* assets) {
	ID3D12Device* device = gpu->GetDevice();

	CreateMeshes(device, assets);
	CreateTextures(device, heap, assets);

	// 全画像のコピーを1回の命令書にまとめて、完了まで待つ。
	gpu->ExecuteImmediately([&](ID3D12GraphicsCommandList* list) {
		for (uint32_t i = 0; i < textures_.size(); i++) {
			TextureHandle handle;
			handle.id = i;
			textures_[i].RecordUpload(list, assets->GetImageData(handle));
		}
	});

	for (Texture& texture : textures_) {
		texture.ReleaseUploadBuffer();
	}
}

GpuMesh& GpuAssetTable::GetMesh(MeshHandle handle) {
	if (handle.id >= meshes_.size()) {
		WarnNotUploaded();
		return meshes_[0];
	}
	return meshes_[handle.id];
}

Texture& GpuAssetTable::GetTexture(TextureHandle handle) {
	if (handle.id >= textures_.size()) {
		WarnNotUploaded();
		return textures_[0];
	}
	return textures_[handle.id];
}

void GpuAssetTable::CreateMeshes(ID3D12Device* device, IAssetLibrary* assets) {
	meshes_.resize(assets->GetMeshCount());

	for (uint32_t i = 0; i < meshes_.size(); i++) {
		MeshHandle handle;
		handle.id = i;
		const MeshData& mesh = assets->GetMeshData(handle);

		meshes_[i].vertexBuffer.Initialize(device, mesh.vertices.data(), sizeof(MeshVertex), static_cast<UINT>(mesh.vertices.size()));
		meshes_[i].indexBuffer.Initialize(device, mesh.indices.data(), static_cast<UINT>(mesh.indices.size()));
	}
}

void GpuAssetTable::CreateTextures(ID3D12Device* device, ShaderResourceHeap* heap, IAssetLibrary* assets) {
	for (uint32_t i = 0; i < assets->GetTextureCount(); i++) {
		if (!heap->HasFreeSlot()) {
			printf("テクスチャの棚が満杯なので、残りは代わりの物で描きます。\n");
			break;
		}

		TextureHandle handle;
		handle.id = i;

		textures_.emplace_back();
		textures_.back().Initialize(device, heap, assets->GetImageData(handle));
	}
}

void GpuAssetTable::WarnNotUploaded() {
	if (hasWarnedNotUploaded_) {
		return;
	}

	printf("転送していないアセットがあるので、代わりの物で描きます。\n");
	hasWarnedNotUploaded_ = true;
}

} // namespace KujataEngine
