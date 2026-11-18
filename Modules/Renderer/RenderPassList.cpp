#include "RenderPassList.h"

#include "Core/Assert.h"

#include <algorithm>

namespace KujataEngine {

void RenderPassList::Initialize(ShaderCompiler* shaderCompiler, ShaderResourceHeap* shaderResourceHeap) {
	shaderCompiler_ = shaderCompiler;
	shaderResourceHeap_ = shaderResourceHeap;
}

void RenderPassList::AddPass(IRenderPass* pass, RenderStage stage, int order) {
	KUJATA_ASSERT(pass != nullptr);

	entries_.push_back({ pass, stage, order });

	// 足すのは起動のときだけなので、足すたびに並べ直し、同じ段階と番号なら足した順を保つ。
	std::stable_sort(entries_.begin(), entries_.end(), [](const Entry& a, const Entry& b) {
		if (a.stage != b.stage) {
			return a.stage < b.stage;
		}
		return a.order < b.order;
	});
}

ShaderCompiler* RenderPassList::GetShaderCompiler() {
	return shaderCompiler_;
}

ShaderResourceHeap* RenderPassList::GetShaderResourceHeap() {
	return shaderResourceHeap_;
}

void RenderPassList::Execute(const RenderContext& context) {
	for (const Entry& entry : entries_) {
		entry.pass->Execute(context);
	}
}

} // namespace KujataEngine
