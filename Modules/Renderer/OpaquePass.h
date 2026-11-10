#pragma once

#include "ConstantBuffer.h"
#include "IRenderPass.h"
#include "Interface/IGpu.h"
#include "PipelineState.h"
#include "ShaderCompiler.h"
#include "ShaderResourceHeap.h"

#include <filesystem>

namespace KujataEngine {

/// <summary>
/// 不透明な物を、依頼ごとにメッシュと画像を切り替えて描く工程。
/// </summary>
class OpaquePass : public IRenderPass {
public:
	/// <summary>
	/// シェーダーやバッファを準備し、用意できなければログを出して、この工程だけ描かない。
	/// </summary>
	void Initialize(IGpu* gpu, ShaderCompiler* shaderCompiler, ShaderResourceHeap* shaderResourceHeap, const std::filesystem::path& dataDirectory);

	void Execute(const RenderContext& context) override;

private:
	bool isReady_ = false;
	bool hasWarnedTooManyDraws_ = false;
	ShaderResourceHeap* shaderResourceHeap_ = nullptr;

	PipelineState pipelineState_;
	ConstantBuffer transformBuffer_;
	ConstantBuffer lightBuffer_;
};

} // namespace KujataEngine
