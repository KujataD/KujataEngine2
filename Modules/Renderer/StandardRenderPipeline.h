#pragma once

#include "IRenderPass.h"
#include "IRenderPipeline.h"

#include <vector>

namespace KujataEngine {

/// <summary>
/// 追加された工程を順番に実行する標準の手順。
/// </summary>
class StandardRenderPipeline : public IRenderPipeline {
public:
	/// <summary>
	/// 工程を最後に足し、工程の持ち主は呼び出し側のままにする。
	/// </summary>
	void AddPass(IRenderPass* pass);

	void Render(const RenderContext& context) override;

private:
	std::vector<IRenderPass*> passes_;
};

} // namespace KujataEngine
