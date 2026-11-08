#include "Texture.h"

#include "Core/Assert.h"
#include "UploadBuffer.h"

#include "../../externals/DirectXTex/DirectXTex.h"
#include "../../externals/DirectXTex/d3dx12.h"

#include <cstdio>
#include <vector>

// d3dx12.hが使うIIDの定義が入っている。
#pragma comment(lib, "dxguid.lib")

using Microsoft::WRL::ComPtr;

namespace KujataEngine {

/// <summary>
/// 画像ファイルを読み、ミップマップまで作って返す。
/// </summary>
static bool LoadImageWithMipMaps(const std::wstring& filePath, DirectX::ScratchImage* mipImages) {
	DirectX::ScratchImage image;
	if (FAILED(DirectX::LoadFromWICFile(filePath.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image))) {
		printf("テクスチャのファイルを読めませんでした: %ls\n", filePath.c_str());
		return false;
	}

	// 1x1の画像はこれ以上小さくできないので、そのまま使う。
	const bool canGenerateMipMaps = image.GetMetadata().width > 1 && image.GetMetadata().height > 1;
	if (!canGenerateMipMaps) {
		const HRESULT result = mipImages->InitializeFromImage(*image.GetImages());
		KUJATA_ASSERT(SUCCEEDED(result));
		return true;
	}

	const HRESULT result = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, *mipImages);
	KUJATA_ASSERT(SUCCEEDED(result));
	return true;
}

/// <summary>
/// GPUの高速なメモリにテクスチャの置き場所を作る。
/// </summary>
static ComPtr<ID3D12Resource> CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata) {
	D3D12_RESOURCE_DESC description = {};
	description.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	description.Width = static_cast<UINT64>(metadata.width);
	description.Height = static_cast<UINT>(metadata.height);
	description.DepthOrArraySize = static_cast<UINT16>(metadata.arraySize);
	description.MipLevels = static_cast<UINT16>(metadata.mipLevels);
	description.Format = metadata.format;
	description.SampleDesc.Count = 1;

	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	ComPtr<ID3D12Resource> resource;
	const HRESULT result = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &description, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&resource));
	KUJATA_ASSERT(SUCCEEDED(result));
	return resource;
}

/// <summary>
/// CPUから書ける一時バッファを経由して、画像をGPUのメモリへコピーする。
/// </summary>
static void UploadTexture(IGpu* gpu, ID3D12Resource* texture, const DirectX::ScratchImage& mipImages) {
	ID3D12Device* device = gpu->GetDevice();

	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	const HRESULT result = DirectX::PrepareUpload(device, mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresources);
	KUJATA_ASSERT(SUCCEEDED(result));

	const UINT subresourceCount = static_cast<UINT>(subresources.size());
	const UINT64 intermediateSize = GetRequiredIntermediateSize(texture, 0, subresourceCount);
	ComPtr<ID3D12Resource> intermediate = CreateUploadBuffer(device, static_cast<UINT>(intermediateSize));

	gpu->ExecuteImmediately([&](ID3D12GraphicsCommandList* list) {
		UpdateSubresources(list, texture, intermediate.Get(), 0, 0, subresourceCount, subresources.data());

		// コピーが終わったら、シェーダーから読める状態にする。
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = texture;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
		list->ResourceBarrier(1, &barrier);
	});
}

bool Texture::Load(IGpu* gpu, ShaderResourceHeap* heap, const std::wstring& filePath) {
	DirectX::ScratchImage mipImages;
	if (!LoadImageWithMipMaps(filePath, &mipImages)) {
		return false;
	}

	ID3D12Device* device = gpu->GetDevice();
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

	resource_ = CreateTextureResource(device, metadata);
	UploadTexture(gpu, resource_.Get(), mipImages);

	D3D12_SHADER_RESOURCE_VIEW_DESC viewDescription = {};
	viewDescription.Format = metadata.format;
	viewDescription.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	viewDescription.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	viewDescription.Texture2D.MipLevels = static_cast<UINT>(metadata.mipLevels);

	const UINT heapIndex = heap->Allocate();
	device->CreateShaderResourceView(resource_.Get(), &viewDescription, heap->GetCpuHandle(heapIndex));
	gpuHandle_ = heap->GetGpuHandle(heapIndex);
	return true;
}

void Texture::Bind(ID3D12GraphicsCommandList* list, UINT rootParameterIndex) {
	list->SetGraphicsRootDescriptorTable(rootParameterIndex, gpuHandle_);
}

} // namespace KujataEngine
