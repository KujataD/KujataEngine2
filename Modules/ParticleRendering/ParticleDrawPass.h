#pragma once

#include "GraphicsKit/ConstantBuffer.h"
#include "GraphicsKit/IRenderPass.h"
#include "GraphicsKit/ShaderCompiler.h"
#include "GraphicsKit/ShaderResourceHeap.h"
#include "Interface/IGpu.h"
#include "Interface/ParticleData.h"
#include "ParticleInstances.h"
#include "ParticleShapeTable.h"

#include <d3d12.h>
#include <filesystem>
#include <vector>
#include <wrl/client.h>

namespace KujataEngine {

/// <summary>
/// 動かした粒子の粒を、粒ごとにカメラへ向いた板で足し合わせて描く工程。
/// </summary>
class ParticleDrawPass : public IRenderPass {
public:
	// RootSignatureの中で、カメラと粒の設定の定数バッファが何番目の入口か。
	static const UINT kConstantsRootParameterIndex = 0;

	// RootSignatureの中で、粒の位置と速度のバッファが何番目の入口か。
	static const UINT kStatesRootParameterIndex = 1;

	// RootSignatureの中で、集まる先のバッファが何番目の入口か。
	static const UINT kTargetsRootParameterIndex = 2;

	/// <summary>
	/// シェーダーと描く設定を準備し、用意できなければログを出して、この工程だけ描かない。依頼と形と粒のバッファはParticleRenderingから借りる。
	/// </summary>
	void Initialize(IGpu* gpu, ShaderCompiler* shaderCompiler, ShaderResourceHeap* shaderResourceHeap, const std::filesystem::path& resourcesDirectory, const std::vector<ParticleRequest>* requests, const ParticleShapeTable* shapes, const ParticleInstances* instances);

	void Execute(const RenderContext& context) override;

private:
	/// <summary>
	/// 頂点の入力を使わず、定数バッファと粒と集まる先のバッファを受け取る入口の一覧を作る。
	/// </summary>
	void CreateRootSignature(ID3D12Device* device);

	/// <summary>
	/// 板の明るさを足し合わせて描く設定をまとめたPSOを作る。
	/// </summary>
	void CreatePipelineState(IGpu* gpu, IDxcBlob* vertexShader, IDxcBlob* pixelShader);

	bool isReady_ = false;
	ShaderResourceHeap* shaderResourceHeap_ = nullptr;
	const std::vector<ParticleRequest>* requests_ = nullptr;
	const ParticleShapeTable* shapes_ = nullptr;
	const ParticleInstances* instances_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
	ConstantBuffer constantBuffer_;
};

} // namespace KujataEngine
