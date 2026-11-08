#pragma once

#include <Windows.h>
#include <d3d12.h>
#include <d3d12shader.h>
#include <dxcapi.h>
#include <wrl/client.h>

namespace KujataEngine {

/// <summary>
/// シェーダーと描き方の設定を1つにまとめたPSOを持つクラス。
/// </summary>
class PipelineState {
public:
	/// <summary>
	/// Root SignatureとPSOを作る。
	/// </summary>
	void Initialize(ID3D12Device* device, IDxcBlob* vertexShader, IDxcBlob* pixelShader, DXGI_FORMAT renderTargetFormat);

	/// <summary>
	/// このPSOで描く設定を命令書に書く。
	/// </summary>
	void Bind(ID3D12GraphicsCommandList* list);

private:
	void CreateRootSignature(ID3D12Device* device);
	void CreatePipelineState(ID3D12Device* device, IDxcBlob* vertexShader, IDxcBlob* pixelShader, DXGI_FORMAT renderTargetFormat);

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
};

} // namespace KujataEngine
