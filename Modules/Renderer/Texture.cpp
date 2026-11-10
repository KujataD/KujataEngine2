#include "Texture.h"

#include "Core/Assert.h"
#include "UploadBuffer.h"

#include "../../externals/DirectXTex/d3dx12.h"

#include <vector>

// d3dx12.hが使うIIDの定義が入っている。
#pragma comment(lib, "dxguid.lib")

namespace KujataEngine {

/// <summary>
/// APIに依存しない画素の形式を、DirectXの形式に変換する。
/// </summary>
static DXGI_FORMAT ToDxgiFormat(ImageFormat format) {
	switch (format) {
	case ImageFormat::Rgba8Srgb:
		return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	}
	return DXGI_FORMAT_UNKNOWN;
}

void Texture::Initialize(ID3D12Device* device, ShaderResourceHeap* heap, const ImageData& image) {
	const DXGI_FORMAT format = ToDxgiFormat(image.format);
	const UINT16 mipCount = static_cast<UINT16>(image.mips.size());

	D3D12_RESOURCE_DESC description = {};
	description.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	description.Width = image.mips[0].width;
	description.Height = image.mips[0].height;
	description.DepthOrArraySize = 1;
	description.MipLevels = mipCount;
	description.Format = format;
	description.SampleDesc.Count = 1;

	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	const HRESULT result = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &description, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&resource_));
	KUJATA_ASSERT(SUCCEEDED(result));

	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(resource_.Get(), 0, mipCount);
	uploadBuffer_ = CreateUploadBuffer(device, static_cast<UINT>(uploadBufferSize));

	D3D12_SHADER_RESOURCE_VIEW_DESC viewDescription = {};
	viewDescription.Format = format;
	viewDescription.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	viewDescription.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	viewDescription.Texture2D.MipLevels = mipCount;

	const UINT heapIndex = heap->Allocate();
	device->CreateShaderResourceView(resource_.Get(), &viewDescription, heap->GetCpuHandle(heapIndex));
	gpuHandle_ = heap->GetGpuHandle(heapIndex);
}

void Texture::RecordUpload(ID3D12GraphicsCommandList* list, const ImageData& image) {
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	for (const ImageMip& mip : image.mips) {
		D3D12_SUBRESOURCE_DATA subresource = {};
		subresource.pData = mip.pixels.data();
		subresource.RowPitch = mip.rowPitch;
		subresource.SlicePitch = mip.pixels.size();
		subresources.push_back(subresource);
	}

	UpdateSubresources(list, resource_.Get(), uploadBuffer_.Get(), 0, 0, static_cast<UINT>(subresources.size()), subresources.data());

	// コピーが終わったら、シェーダーから読める状態にする。
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = resource_.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	list->ResourceBarrier(1, &barrier);
}

void Texture::ReleaseUploadBuffer() {
	uploadBuffer_.Reset();
}

void Texture::Bind(ID3D12GraphicsCommandList* list, UINT rootParameterIndex) {
	list->SetGraphicsRootDescriptorTable(rootParameterIndex, gpuHandle_);
}

} // namespace KujataEngine
