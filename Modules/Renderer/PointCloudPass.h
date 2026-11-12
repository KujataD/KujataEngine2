#pragma once

#include "ConstantBuffer.h"
#include "IRenderPass.h"
#include "Interface/IGpu.h"
#include "ShaderCompiler.h"
#include "ShaderResourceHeap.h"

#include <d3d12.h>
#include <filesystem>
#include <wrl/client.h>

namespace KujataEngine {

/// <summary>
///	点を板で書くための工程。
/// </summary>
class PointCloudPass : public IRenderPass {
public:
	// RootSignature中、カメラの定数バッファが何番目か。
	static const UINT kConstantsRootParameterIndex = 0;

	// RootSignature中、点のバッファが何番目か。
	static const UINT kPointsRootParameterIndex = 1;

	/// <summary>
	/// シェーダーと点のバッファを準備し、用意できなければログを出して、この工程だけ描かない。
	/// </summary>
	void Initialize(IGpu* gpu, ShaderCompiler* shaderCompiler, ShaderResourceHeap* shaderResourceHeap, const std::filesystem::path& dataDirectory);

	void Execute(const RenderContext& context) override;

private:
	/// <summary>
	/// 格子に並べた点を、GPUだけが読み書きするバッファへ送り、シェーダーから読む名札を作る。
	/// </summary>
	void CreatePointBuffer(IGpu* gpu);

	/// <summary>
	/// 頂点の入力を使わず、定数バッファと点のバッファだけを受け取る入口の一覧を作る。
	/// </summary>
	void CreateRootSignature(ID3D12Device* device);

	/// <summary>
	/// 板を描く設定をまとめたPSOを作る。
	/// </summary>
	void CreatePipelineState(IGpu* gpu, IDxcBlob* vertexShader, IDxcBlob* pixelShader);

	bool isReady_ = false;
	ShaderResourceHeap* shaderResourceHeap_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> pointBuffer_;
	D3D12_GPU_DESCRIPTOR_HANDLE pointsGpuHandle_ = {};

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
	ConstantBuffer constantBuffer_;
};

} // namespace KujataEngine
