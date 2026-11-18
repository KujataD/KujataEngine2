#pragma once

#include "GpuAssetTable.h"
#include "GraphicsKit/ConstantBuffer.h"
#include "GraphicsKit/IRenderPass.h"
#include "GraphicsKit/IRenderPasses.h"
#include "GraphicsKit/ShaderCompiler.h"
#include "GraphicsKit/ShaderResourceHeap.h"
#include "Interface/IGpu.h"
#include "RenderQueue.h"

#include <d3d12.h>
#include <filesystem>
#include <wrl/client.h>

namespace KujataEngine {

/// <summary>
/// 不透明な物を、依頼ごとにメッシュと画像を切り替えて描く工程。
/// </summary>
class OpaquePass : public IRenderPass {
public:
	// RootSignatureの中で、行列の定数バッファが何番目の入口か。
	static const UINT kTransformRootParameterIndex = 0;

	// RootSignatureの中で、テクスチャが何番目の入口か。
	static const UINT kTextureRootParameterIndex = 1;

	// RootSignatureの中で、光の定数バッファが何番目の入口か。
	static const UINT kLightRootParameterIndex = 2;

	/// <summary>
	/// シェーダーと描く設定とバッファを準備し、用意できなければログを出して、この工程だけ描かない。
	/// シェーダーのコンパイラと棚は登録口から、描く物の依頼とGPUに上げた資産はRendererから借りる。
	/// </summary>
	void Initialize(IGpu* gpu, IRenderPasses* renderPasses, const std::filesystem::path& resourcesDirectory, const RenderQueue* renderQueue, GpuAssetTable* gpuAssets);

	void Execute(const RenderContext& context) override;

private:
	/// <summary>
	/// 行列と光の定数バッファと、テクスチャの表を受け取る入口の一覧を作る。
	/// </summary>
	void CreateRootSignature(ID3D12Device* device);

	/// <summary>
	/// メッシュの頂点を読み、深度を書いて描く設定をまとめたPSOを作る。
	/// </summary>
	void CreatePipelineState(IGpu* gpu, IDxcBlob* vertexShader, IDxcBlob* pixelShader);

	bool isReady_ = false;
	bool hasWarnedTooManyDraws_ = false;
	ShaderResourceHeap* shaderResourceHeap_ = nullptr;
	const RenderQueue* renderQueue_ = nullptr;
	GpuAssetTable* gpuAssets_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
	ConstantBuffer transformBuffer_;
	ConstantBuffer lightBuffer_;
};

} // namespace KujataEngine
