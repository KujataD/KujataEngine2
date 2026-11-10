#pragma once

#include "Interface/AssetData.h"
#include "ShaderResourceHeap.h"

#include <d3d12.h>
#include <wrl/client.h>

namespace KujataEngine {

/// <summary>
/// 画像のデータをGPUへ転送して、シェーダーから使えるようにしたテクスチャ。
/// </summary>
class Texture {
public:
	/// <summary>
	/// GPUのメモリと転送用の一時バッファと名札を作る。
	/// </summary>
	void Initialize(ID3D12Device* device, ShaderResourceHeap* heap, const ImageData& image);

	/// <summary>
	/// 画像をGPUのメモリへコピーする命令を書く。
	/// </summary>
	void RecordUpload(ID3D12GraphicsCommandList* list, const ImageData& image);

	/// <summary>
	/// コピーが終わった後に一時バッファを捨てる。
	/// </summary>
	void ReleaseUploadBuffer();

	/// <summary>
	/// このテクスチャを使う設定を命令書に書く。
	/// </summary>
	void Bind(ID3D12GraphicsCommandList* list, UINT rootParameterIndex);

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> resource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer_;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle_ = {};
};

} // namespace KujataEngine
