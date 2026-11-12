#include "GpuBuffer.h"

#include "Core/Assert.h"

using Microsoft::WRL::ComPtr;

namespace KujataEngine {

ComPtr<ID3D12Resource> CreateGpuBuffer(ID3D12Device* device, UINT64 sizeInBytes, D3D12_RESOURCE_FLAGS flags) {
	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_RESOURCE_DESC description = {};
	description.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	description.Width = sizeInBytes;
	description.Height = 1;
	description.DepthOrArraySize = 1;
	description.MipLevels = 1;
	description.SampleDesc.Count = 1;
	description.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	description.Flags = flags;

	// バッファはCOMMONで作れば、コピーや読み込みのときにGPUが必要な状態へ自動で切り替える。
	ComPtr<ID3D12Resource> buffer;
	const HRESULT result = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &description, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&buffer));
	KUJATA_ASSERT(SUCCEEDED(result));
	return buffer;
}

} // namespace KujataEngine
