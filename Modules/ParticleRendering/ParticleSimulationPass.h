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
/// 登録を待つ形をGPUへ転送し、頼まれた粒子の粒をコンピュートシェーダーで動かす工程。
/// </summary>
class ParticleSimulationPass : public IRenderPass {
public:
	// RootSignatureの中で、定数バッファが何番目の入口か。
	static const UINT kConstantsRootParameterIndex = 0;

	// RootSignatureの中で、書き込む粒のバッファが何番目の入口か。
	static const UINT kStatesRootParameterIndex = 1;

	// RootSignatureの中で、集まる先のバッファが何番目の入口か。
	static const UINT kTargetsRootParameterIndex = 2;

	/// <summary>
	/// シェーダーと設定を準備し、用意できなければログを出して、この工程だけ動かさない。依頼と形と粒のバッファはParticleRenderingから借りる。
	/// </summary>
	void Initialize(IGpu* gpu, ShaderCompiler* shaderCompiler, ShaderResourceHeap* shaderResourceHeap, const std::filesystem::path& resourcesDirectory, const std::vector<ParticleRequest>* requests, ParticleShapeTable* shapes, ParticleInstances* instances);

	void Execute(const RenderContext& context) override;

private:
	/// <summary>
	/// 粒のバッファの状態を、まとめて切り替える命令を書く。
	/// </summary>
	void TransitionStates(ID3D12GraphicsCommandList* list, const std::vector<ParticleInstance*>& instances, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);

	/// <summary>
	/// 定数バッファと、粒と集まる先のバッファを受け取る入口の一覧を作る。
	/// </summary>
	void CreateRootSignature(ID3D12Device* device);

	/// <summary>
	/// 粒を動かすコンピュートシェーダーのPSOを作る。
	/// </summary>
	void CreatePipelineState(ID3D12Device* device, IDxcBlob* computeShader);

	bool isReady_ = false;
	ShaderResourceHeap* shaderResourceHeap_ = nullptr;
	const std::vector<ParticleRequest>* requests_ = nullptr;
	ParticleShapeTable* shapes_ = nullptr;
	ParticleInstances* instances_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
	ConstantBuffer constantBuffer_;

	// 粒の揺れの進み具合を表す角度で、単位はラジアン。
	float floatPhase_ = 0.0f;
};

} // namespace KujataEngine
