#pragma once

#include "GraphicsKit/ConstantBuffer.h"
#include "GraphicsKit/IRenderPass.h"
#include "GraphicsKit/IRenderPasses.h"
#include "GraphicsKit/ShaderCompiler.h"
#include "GraphicsKit/ShaderResourceHeap.h"
#include "Interface/IGpu.h"
#include "LineRendering.h"

#include <d3d12.h>
#include <filesystem>
#include <wrl/client.h>

namespace KujataEngine {

/// <summary>
/// 太い線の一覧1つ分の、CPUから書けるバッファと、シェーダーが読むための棚の名札。
/// </summary>
struct ThickLineList {
	Microsoft::WRL::ComPtr<ID3D12Resource> buffer;

	// 作った後はずっと書き込める状態にしておく。
	ThickLineSegment* segments = nullptr;

	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = {};
};

/// <summary>
/// 預かった線を、細い線はLINELISTで、太い線はカメラに向いた板で描く工程。
/// </summary>
class LinePass : public IRenderPass {
public:
	// RootSignatureの中で、カメラの定数バッファが何番目の入口か。
	static const UINT kConstantsRootParameterIndex = 0;

	// RootSignatureの中で、太い線の一覧の表が何番目の入口か。
	static const UINT kThickLinesRootParameterIndex = 1;

	/// <summary>
	/// シェーダーと描く設定とバッファを準備し、用意できなければログを出して、この工程だけ描かない。
	/// シェーダーのコンパイラと棚は登録口から、描く線はLineRenderingから借りる。
	/// </summary>
	void Initialize(IGpu* gpu, IRenderPasses* renderPasses, const std::filesystem::path& resourcesDirectory, const LineBatches* batches);

	void Execute(const RenderContext& context) override;

private:
	/// <summary>
	/// 定数バッファと、太い線の一覧の表を受け取る入口の一覧を作る。
	/// </summary>
	void CreateRootSignature(ID3D12Device* device);

	/// <summary>
	/// 太い線を上限の本数まで入れられるバッファと、棚の名札を作る。
	/// </summary>
	ThickLineList CreateThickLineList(ID3D12Device* device);

	/// <summary>
	/// 細い線か太い線かと、深度テストの有無に合わせたPSOを作る。
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12PipelineState> CreatePipelineState(IGpu* gpu, IDxcBlob* vertexShader, IDxcBlob* pixelShader, bool isThin, bool depthTest);

	bool isReady_ = false;
	const LineBatches* batches_ = nullptr;
	ShaderResourceHeap* shaderResourceHeap_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> thinDepthTestedState_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> thinAlwaysVisibleState_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> thickDepthTestedState_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> thickAlwaysVisibleState_;

	// CPUから毎フレーム書き込むバッファで、作った後はずっと書き込める状態にしておく。
	Microsoft::WRL::ComPtr<ID3D12Resource> thinVertexBuffer_;
	LineVertex* thinVertices_ = nullptr;

	// 名札は始まりの位置が決まっているので、深度テストの有無で一覧を分け、どちらも頭から詰める。
	ThickLineList thickDepthTested_;
	ThickLineList thickAlwaysVisible_;

	ConstantBuffer constantBuffer_;
};

} // namespace KujataEngine
