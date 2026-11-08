#pragma once

#include "Interface/IGpu.h"
#include "ShaderResourceHeap.h"

#include <d3d12.h>
#include <string>
#include <wrl/client.h>

namespace KujataEngine {

/// <summary>
/// 画像ファイルを読み込んで、シェーダーから使えるようにしたテクスチャ。
/// </summary>
class Texture {
public:
	/// <summary>
	/// 画像を読み込んでGPUへ転送し、読めなければログを出してfalseを返す。
	/// </summary>
	bool Load(IGpu* gpu, ShaderResourceHeap* heap, const std::wstring& filePath);

	/// <summary>
	/// このテクスチャを使う設定を命令書に書く。
	/// </summary>
	void Bind(ID3D12GraphicsCommandList* list, UINT rootParameterIndex);

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> resource_;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle_ = {};
};

} // namespace KujataEngine
