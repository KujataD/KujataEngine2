#pragma once

#include "ConstantBuffer.h"
#include "IRenderPass.h"
#include "IndexBuffer.h"
#include "Interface/IGpu.h"
#include "PipelineState.h"
#include "ShaderCompiler.h"
#include "ShaderResourceHeap.h"
#include "Texture.h"
#include "VertexBuffer.h"

#include <filesystem>

namespace KujataEngine {

/// <summary>
/// 不透明な物を描く工程で、今は立方体を描く。
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
	ShaderResourceHeap* shaderResourceHeap_ = nullptr;

	PipelineState pipelineState_;
	VertexBuffer vertexBuffer_;
	IndexBuffer indexBuffer_;
	ConstantBuffer transformBuffer_;
	Texture texture_;
};

} // namespace KujataEngine
