#include "ParticleRendering.h"

#include "Core/Assert.h"
#include "GraphicsKit/IRenderPasses.h"
#include "Interface/IAssetLibrary.h"
#include "Interface/IGpu.h"
#include "ParticleDrawPass.h"
#include "ParticleInstances.h"
#include "ParticleShapeTable.h"
#include "ParticleSimulationPass.h"

#include <cstdio>

namespace KujataEngine {

ParticleRendering::ParticleRendering() : shapes_(std::make_unique<ParticleShapeTable>()), instances_(std::make_unique<ParticleInstances>()), simulationPass_(std::make_unique<ParticleSimulationPass>()), drawPass_(std::make_unique<ParticleDrawPass>()) {}

// unique_ptrが中身を消すには中身が見えている必要があるので、ここで定義する。
ParticleRendering::~ParticleRendering() = default;

std::vector<std::string> ParticleRendering::GetProvidedServices() {
	return { kParticleRendererServiceName };
}

std::vector<std::string> ParticleRendering::GetRequiredServices() {
	return { kRenderPassesServiceName, kGpuServiceName, kAssetLibraryServiceName };
}

void ParticleRendering::Startup(ServiceRegistry& services, FrameLoop& frameLoop) {
	IRenderPasses* renderPasses = static_cast<IRenderPasses*>(services.Get(kRenderPassesServiceName));
	KUJATA_ASSERT(renderPasses != nullptr);

	IGpu* gpu = static_cast<IGpu*>(services.Get(kGpuServiceName));
	KUJATA_ASSERT(gpu != nullptr);

	IAssetLibrary* assets = static_cast<IAssetLibrary*>(services.Get(kAssetLibraryServiceName));
	KUJATA_ASSERT(assets != nullptr);

	ShaderResourceHeap* shaderResourceHeap = renderPasses->GetShaderResourceHeap();
	shapes_->Initialize(gpu->GetDevice(), shaderResourceHeap);
	instances_->Initialize(gpu->GetDevice(), shaderResourceHeap);

	// 動かしてから描くように、同じ段階の中で動かす工程を先に並べる。
	simulationPass_->Initialize(gpu, renderPasses->GetShaderCompiler(), shaderResourceHeap, assets->GetEngineResourcesDirectory(), &requests_, shapes_.get(), instances_.get());
	drawPass_->Initialize(gpu, renderPasses->GetShaderCompiler(), shaderResourceHeap, assets->GetEngineResourcesDirectory(), &requests_, shapes_.get(), instances_.get());
	renderPasses->AddPass(simulationPass_.get(), RenderStage::Additive, 0);
	renderPasses->AddPass(drawPass_.get(), RenderStage::Additive, 1);

	// 受け取る側がIParticleRenderer*に戻せるように、void*にする前にIParticleRenderer*へ変換する。
	services.Register(kParticleRendererServiceName, static_cast<IParticleRenderer*>(this));

	// Rendererより後に起動するので、同じ段階ではRendererの描画の後に呼ばれる。
	// 描いた後に空にするので、カメラがなく描かなかったフレームでも依頼が溜まらない。
	frameLoop.Register(kRender, [this]() {
		requests_.clear();
		instances_->RemoveUnused();
	});
}

ParticleShapeHandle ParticleRendering::AddShape(const ParticleShape& shape) {
	return shapes_->Add(shape);
}

void ParticleRendering::SubmitParticles(const ParticleRequest& request) {
	// 粒子の数はゲームしだいで増えるので、止めずに上限までだけを描く。
	if (requests_.size() >= kMaxParticleRequestCount) {
		if (!hasWarnedTooManyRequests_) {
			printf("粒子が%u個を超えたので、超えた分は描きません。\n", kMaxParticleRequestCount);
			hasWarnedTooManyRequests_ = true;
		}
		return;
	}

	requests_.push_back(request);
}

} // namespace KujataEngine
