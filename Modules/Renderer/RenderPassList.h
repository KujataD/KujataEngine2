#pragma once

#include "GraphicsKit/IRenderPasses.h"

#include <vector>

namespace KujataEngine {

/// <summary>
/// 登録された工程を、段階と番号の順に並べて持ち、その順に実行する。
/// </summary>
class RenderPassList : public IRenderPasses {
public:
	/// <summary>
	/// 工程に貸す共有の部品を覚える。
	/// </summary>
	void Initialize(ShaderCompiler* shaderCompiler, ShaderResourceHeap* shaderResourceHeap);

	void AddPass(IRenderPass* pass, RenderStage stage, int order) override;
	ShaderCompiler* GetShaderCompiler() override;
	ShaderResourceHeap* GetShaderResourceHeap() override;

	/// <summary>
	/// 並んだ順に、全部の工程を実行する。
	/// </summary>
	void Execute(const RenderContext& context);

private:
	struct Entry {
		IRenderPass* pass;
		RenderStage stage;
		int order;
	};

	std::vector<Entry> entries_;
	ShaderCompiler* shaderCompiler_ = nullptr;
	ShaderResourceHeap* shaderResourceHeap_ = nullptr;
};

} // namespace KujataEngine
