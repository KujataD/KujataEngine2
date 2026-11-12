#pragma once

#include "ConstantBuffer.h"
#include "GpuTimer.h"
#include "IRenderPass.h"
#include "Interface/IGpu.h"
#include "ShaderCompiler.h"
#include "ShaderResourceHeap.h"

#include <cstdint>
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

	// 点を動かすRootSignature中、ルート定数が何番目か。
	static const UINT kUpdateConstantsRootParameterIndex = 0;

	// 点を動かすRootSignature中、書き込む点のバッファが何番目か。
	static const UINT kUpdatePointsRootParameterIndex = 1;

	/// <summary>
	/// シェーダーと点のバッファを準備し、用意できなければログを出して、この工程だけ描かない。
	/// </summary>
	void Initialize(IGpu* gpu, ShaderCompiler* shaderCompiler, ShaderResourceHeap* shaderResourceHeap, const std::filesystem::path& dataDirectory);

	void Execute(const RenderContext& context) override;

private:
	/// <summary>
	/// コンピュートシェーダーで点を動かし、描画で読める状態にする命令を書く。
	/// </summary>
	void UpdatePoints(const RenderContext& context);

	/// <summary>
	/// 前のフレームのGPU時間を足し込み、決めたフレーム数ごとに平均をログに出す。
	/// </summary>
	void ReportGpuTime();

	/// <summary>
	/// 点のバッファの状態を切り替える命令を書く。
	/// </summary>
	void TransitionPointBuffer(ID3D12GraphicsCommandList* list, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);

	/// <summary>
	/// 格子に並べた点を、GPUだけが読み書きするバッファへ送り、シェーダーから読む名札と書く名札を作る。
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

	/// <summary>
	/// 点を動かすコンピュートシェーダーが、ルート定数と書き込む点のバッファを受け取る入口の一覧を作る。
	/// </summary>
	void CreateUpdateRootSignature(ID3D12Device* device);

	/// <summary>
	/// 点を動かすコンピュートシェーダーのPSOを作る。
	/// </summary>
	void CreateUpdatePipelineState(ID3D12Device* device, IDxcBlob* computeShader);

	bool isReady_ = false;
	ShaderResourceHeap* shaderResourceHeap_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> pointBuffer_;
	D3D12_GPU_DESCRIPTOR_HANDLE pointsGpuHandle_ = {};
	D3D12_GPU_DESCRIPTOR_HANDLE pointsUavGpuHandle_ = {};

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
	ConstantBuffer constantBuffer_;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> updateRootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> updatePipelineState_;

	// 乱数を毎フレーム変えるための、点を動かした回数。
	uint32_t frameIndex_ = 0;

	GpuTimer gpuTimer_;

	// 平均を出すために足し込んだGPU時間で、単位はミリ秒。
	double updateMillisecondsSum_ = 0.0;
	double drawMillisecondsSum_ = 0.0;
	uint32_t measuredFrameCount_ = 0;
};

} // namespace KujataEngine
