#pragma once

#include "GraphicsKit/ConstantBuffer.h"
#include "GraphicsKit/GpuTimer.h"
#include "GraphicsKit/ShaderCompiler.h"
#include "GraphicsKit/ShaderResourceHeap.h"
#include "IRenderPass.h"
#include "Interface/IGpu.h"

#include <cstdint>
#include <d3d12.h>
#include <filesystem>
#include <map>
#include <vector>
#include <wrl/client.h>

namespace KujataEngine {

/// <summary>
/// 描く物1つ分の、点ごとの位置と速度と形になる度合いを持つバッファと名札。
/// </summary>
struct PointCloudInstance {
	Microsoft::WRL::ComPtr<ID3D12Resource> stateBuffer;

	// 棚の中の場所で、読む名札と書く名札の2つを持つ。
	UINT readHeapIndex = 0;
	UINT writeHeapIndex = 0;

	UINT pointCount = 0;

	// 作ったばかりのバッファには何も入っていないので、最初の更新で点を散った位置に置く。
	bool shouldInitialize = true;

	// このフレームで依頼が来たかどうかで、来なくなった物を片付けるのに使う。
	bool isUsedThisFrame = false;
};

/// <summary>
/// 依頼された点群の点をコンピュートシェーダーで動かし、点ごとの板で描く工程。
/// </summary>
class PointCloudPass : public IRenderPass {
public:
	// 描くRootSignature中、カメラと点の設定の定数バッファが何番目か。
	static const UINT kConstantsRootParameterIndex = 0;

	// 描くRootSignature中、点の位置と速度のバッファが何番目か。
	static const UINT kPointsRootParameterIndex = 1;

	// 描くRootSignature中、集まる先のバッファが何番目か。
	static const UINT kTargetsRootParameterIndex = 2;

	// 点を動かすRootSignature中、定数バッファが何番目か。
	static const UINT kUpdateConstantsRootParameterIndex = 0;

	// 点を動かすRootSignature中、書き込む点のバッファが何番目か。
	static const UINT kUpdatePointsRootParameterIndex = 1;

	// 点を動かすRootSignature中、集まる先のバッファが何番目か。
	static const UINT kUpdateTargetsRootParameterIndex = 2;

	/// <summary>
	/// シェーダーと描く設定を準備し、用意できなければログを出して、この工程だけ描かない。
	/// </summary>
	void Initialize(IGpu* gpu, ShaderCompiler* shaderCompiler, ShaderResourceHeap* shaderResourceHeap, const std::filesystem::path& dataDirectory);

	void Execute(const RenderContext& context) override;

private:
	/// <summary>
	/// 依頼の番号に対応する点のバッファを返し、初めての番号なら集まる先と同じ数の点で作る。
	/// </summary>
	PointCloudInstance& FindOrCreateInstance(const PointCloudRequest& request, UINT targetCount);

	/// <summary>
	/// コンピュートシェーダーで1つの点群の点を動かし、描画で読める状態にする命令を書く。
	/// </summary>
	void UpdatePoints(const RenderContext& context, const PointCloudRequest& request, PointCloudInstance& instance, UINT constantsIndex);

	/// <summary>
	/// 1つの点群の点を、点ごとの板で描く命令を書く。
	/// </summary>
	void DrawPoints(const RenderContext& context, const PointCloudRequest& request, const PointCloudInstance& instance, UINT constantsIndex);

	/// <summary>
	/// このフレームで依頼が来なかった点のバッファを捨て、棚の場所を使い回せるようにする。
	/// </summary>
	void RemoveUnusedInstances();

	/// <summary>
	/// 棚の場所を1つ確保し、捨てた物の場所があればそれを使い回す。
	/// </summary>
	UINT AllocateHeapIndex();

	/// <summary>
	/// 前のフレームのGPU時間を足し込み、決めたフレーム数ごとに平均をログに出す。
	/// </summary>
	void ReportGpuTime();

	/// <summary>
	/// 点のバッファの状態を切り替える命令を書く。
	/// </summary>
	void TransitionStateBuffer(ID3D12GraphicsCommandList* list, const PointCloudInstance& instance, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);

	/// <summary>
	/// 頂点の入力を使わず、定数バッファと点と集まる先のバッファを受け取る入口の一覧を作る。
	/// </summary>
	void CreateRootSignature(ID3D12Device* device);

	/// <summary>
	/// 板を描く設定をまとめたPSOを作る。
	/// </summary>
	void CreatePipelineState(IGpu* gpu, IDxcBlob* vertexShader, IDxcBlob* pixelShader);

	/// <summary>
	/// 点を動かすコンピュートシェーダーが、定数バッファと点と集まる先のバッファを受け取る入口の一覧を作る。
	/// </summary>
	void CreateUpdateRootSignature(ID3D12Device* device);

	/// <summary>
	/// 点を動かすコンピュートシェーダーのPSOを作る。
	/// </summary>
	void CreateUpdatePipelineState(ID3D12Device* device, IDxcBlob* computeShader);

	bool isReady_ = false;
	IGpu* gpu_ = nullptr;
	ShaderResourceHeap* shaderResourceHeap_ = nullptr;

	// 依頼の番号から、その物の点のバッファを引く。
	std::map<uint64_t, PointCloudInstance> instances_;

	// 捨てた点のバッファが使っていた棚の場所。
	std::vector<UINT> reusableHeapIndices_;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
	ConstantBuffer constantBuffer_;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> updateRootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> updatePipelineState_;
	ConstantBuffer updateConstantBuffer_;

	// 点の揺れの進み具合を表す角度で、単位はラジアン。
	float floatPhase_ = 0.0f;

	GpuTimer gpuTimer_;

	// 平均を出すために足し込んだGPU時間で、単位はミリ秒。
	double updateMillisecondsSum_ = 0.0;
	double drawMillisecondsSum_ = 0.0;
	uint32_t measuredFrameCount_ = 0;

	// 前のフレームで動かして描いた点の数で、GPU時間と一緒にログに出す。
	UINT lastPointCount_ = 0;

	bool hasWarnedTooManyPointClouds_ = false;
	bool hasWarnedTooManyPoints_ = false;
};

} // namespace KujataEngine
