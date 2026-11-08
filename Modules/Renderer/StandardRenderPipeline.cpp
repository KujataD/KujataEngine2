#include "StandardRenderPipeline.h"

#include "Core/Assert.h"

namespace KujataEngine {

void StandardRenderPipeline::AddPass(IRenderPass* pass) {
	KUJATA_ASSERT(pass != nullptr);

	passes_.push_back(pass);
}

void StandardRenderPipeline::Render(const RenderContext& context) {
	for (IRenderPass* pass : passes_) {
		pass->Execute(context);
	}
}

} // namespace KujataEngine
