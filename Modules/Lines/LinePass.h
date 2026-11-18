#pragma once

#include "GraphicsKit/ConstantBuffer.h"
#include "GraphicsKit/IRenderPass.h"
#include "GraphicsKit/ShaderCompiler.h"
#include "Interface/IGpu.h"
#include "Lines.h"

#include <d3d12.h>
#include <filesystem>
#include <wrl/client.h>

namespace KujataEngine {

/// <summary>
/// 預かった線を、細い線はLINELISTで、太い線はカメラに向いた板で描く工程。
/// </summary>
class LinePass : public IRenderPass {
public:
	// RootSignatureの中で、カメラの定数バッファが何番目の入口か。
	static const UINT kConstantsRootParameterIndex = 0;

	// RootSignatureの中で、太い線のバッファが何番目の入口か。
	static const UINT kThickLinesRootParameterIndex = 1;

	/// <summary>
	/// シェーダーと描く設定とバッファを準備し、用意できなければログを出して、この工程だけ描かない。描く線はLinesから借りる。
	/// </summary>
	void Initialize(IGpu* gpu, ShaderCompiler* shaderCompiler, const std::filesystem::path& resourcesDirectory, const LineBatches* batches);

	void Execute(const RenderContext& context) override;

private:
	/// <summary>
	/// 定数バッファと、太い線のバッファを直接受け取る入口の一覧を作る。
	/// </summary>
	void CreateRootSignature(ID3D12Device* device);

	/// <summary>
	/// 細い線か太い線かと、深度テストの有無に合わせたPSOを作る。
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12PipelineState> CreatePipelineState(IGpu* gpu, IDxcBlob* vertexShader, IDxcBlob* pixelShader, bool isThin, bool depthTest);

	bool isReady_ = false;
	const LineBatches* batches_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> thinDepthTestedState_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> thinAlwaysVisibleState_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> thickDepthTestedState_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> thickAlwaysVisibleState_;

	// CPUから毎フレーム書き込むバッファで、作った後はずっと書き込める状態にしておく。
	Microsoft::WRL::ComPtr<ID3D12Resource> thinVertexBuffer_;
	LineVertex* thinVertices_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> thickLineBuffer_;
	ThickLineSegment* thickLines_ = nullptr;

	ConstantBuffer constantBuffer_;
};

} // namespace KujataEngine
