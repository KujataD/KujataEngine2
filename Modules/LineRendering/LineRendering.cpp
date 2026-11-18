#include "LineRendering.h"

#include "Core/Assert.h"
#include "GraphicsKit/IRenderPasses.h"
#include "Interface/IAssetLibrary.h"
#include "Interface/IGpu.h"
#include "LinePass.h"

#include <cstdio>

namespace KujataEngine {

void LineBatches::Clear() {
	thinDepthTested.clear();
	thinAlwaysVisible.clear();
	thickDepthTested.clear();
	thickAlwaysVisible.clear();
}

LineRendering::LineRendering() : pass_(std::make_unique<LinePass>()) {}

// unique_ptrがLinePassを消すには中身が見えている必要があるので、ここで定義する。
LineRendering::~LineRendering() = default;

std::vector<std::string> LineRendering::GetProvidedServices() {
	return { kLineRendererServiceName };
}

std::vector<std::string> LineRendering::GetRequiredServices() {
	return { kRenderPassesServiceName, kGpuServiceName, kAssetLibraryServiceName };
}

void LineRendering::Startup(ServiceRegistry& services, FrameLoop& frameLoop) {
	IRenderPasses* renderPasses = static_cast<IRenderPasses*>(services.Get(kRenderPassesServiceName));
	KUJATA_ASSERT(renderPasses != nullptr);

	IGpu* gpu = static_cast<IGpu*>(services.Get(kGpuServiceName));
	KUJATA_ASSERT(gpu != nullptr);

	IAssetLibrary* assets = static_cast<IAssetLibrary*>(services.Get(kAssetLibraryServiceName));
	KUJATA_ASSERT(assets != nullptr);

	pass_->Initialize(gpu, renderPasses->GetShaderCompiler(), assets->GetEngineResourcesDirectory(), &batches_);
	renderPasses->AddPass(pass_.get(), RenderStage::Debug, 0);

	// 受け取る側がILineRenderer*に戻せるように、void*にする前にILineRenderer*へ変換する。
	services.Register(kLineRendererServiceName, static_cast<ILineRenderer*>(this));

	// Rendererより後に起動するので、同じ段階ではRendererの描画の後に呼ばれる。
	// 描いた後に空にするので、描画より後に頼んだ線は次のフレームに描かれ、カメラがなく描かなかったフレームでも溜まらない。
	frameLoop.Register(kRender, [this]() {
		batches_.Clear();
	});
}

void LineRendering::SubmitLine(const Vector3& start, const Vector3& end, const Vector4& color, bool depthTest) {
	// 頂点は1本に2つなので、頂点の数の半分が線の数になる。
	const size_t lineCount = (batches_.thinDepthTested.size() + batches_.thinAlwaysVisible.size()) / 2;

	// 線の数はゲームしだいで増えるので、止めずに上限までだけを描く。
	if (lineCount >= kMaxThinLineCount) {
		if (!hasWarnedTooManyThinLines_) {
			printf("細い線が%u本を超えたので、超えた分は描きません。\n", kMaxThinLineCount);
			hasWarnedTooManyThinLines_ = true;
		}
		return;
	}

	std::vector<LineVertex>& vertices = depthTest ? batches_.thinDepthTested : batches_.thinAlwaysVisible;
	vertices.push_back({ start, color });
	vertices.push_back({ end, color });
}

void LineRendering::SubmitThickLine(const Vector3& start, const Vector3& end, const Vector4& color, float thickness, LineWidthUnit unit, bool depthTest) {
	const size_t lineCount = batches_.thickDepthTested.size() + batches_.thickAlwaysVisible.size();
	if (lineCount >= kMaxThickLineCount) {
		if (!hasWarnedTooManyThickLines_) {
			printf("太い線が%u本を超えたので、超えた分は描きません。\n", kMaxThickLineCount);
			hasWarnedTooManyThickLines_ = true;
		}
		return;
	}

	ThickLineSegment segment = {};
	segment.start = start;
	segment.thickness = thickness;
	segment.end = end;
	segment.unit = (unit == LineWidthUnit::Pixel) ? 1u : 0u;
	segment.color = color;

	std::vector<ThickLineSegment>& segments = depthTest ? batches_.thickDepthTested : batches_.thickAlwaysVisible;
	segments.push_back(segment);
}

} // namespace KujataEngine
