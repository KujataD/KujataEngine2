#include "RenderSystem.h"

#include "Core/Assert.h"
#include "Core/Math/MathUtil.h"
#include "GraphicsKit/IRenderPasses.h"
#include "Interface/IRenderable.h"

#include <cstdio>

namespace KujataEngine {

// テクスチャの棚に置ける数。
static const UINT kShaderResourceHeapCapacity = 4096;

std::vector<std::string> RenderSystem::GetProvidedServices() {
	return { kRenderPassesServiceName };
}

std::vector<std::string> RenderSystem::GetRequiredServices() {
	return { kGpuServiceName, kSceneServiceName, kAssetLibraryServiceName, kTimeServiceName };
}

void RenderSystem::Startup(ServiceRegistry& services, FrameLoop& frameLoop) {
	gpu_ = static_cast<IGpu*>(services.Get(kGpuServiceName));
	KUJATA_ASSERT(gpu_ != nullptr);

	scene_ = static_cast<Scene*>(services.Get(kSceneServiceName));
	KUJATA_ASSERT(scene_ != nullptr);

	assets_ = static_cast<IAssetLibrary*>(services.Get(kAssetLibraryServiceName));
	KUJATA_ASSERT(assets_ != nullptr);

	time_ = static_cast<ITime*>(services.Get(kTimeServiceName));
	KUJATA_ASSERT(time_ != nullptr);

	shaderCompiler_.Initialize();
	shaderResourceHeap_.Initialize(gpu_->GetDevice(), kShaderResourceHeapCapacity);

	// 工程のモジュールが起動のときに登録できるように、登録口を先に出す。
	passList_.Initialize(&shaderCompiler_, &shaderResourceHeap_);
	services.Register(kRenderPassesServiceName, static_cast<IRenderPasses*>(&passList_));

	// シェーダーは、エンジン既定のリソースのフォルダの Shaders に置く。
	const std::filesystem::path resourcesDirectory = assets_->GetEngineResourcesDirectory();

	opaquePass_.Initialize(gpu_, &passList_, resourcesDirectory, &renderQueue_, &gpuAssets_);
	passList_.AddPass(&opaquePass_, RenderStage::Opaque, 0);


	frameLoop.Register(kSubmitRender, [this]() {
		CollectRenderRequests();
	});
	frameLoop.Register(kRender, [this]() {
		Draw();
	});
}

void RenderSystem::Prepare() {
	gpuAssets_.UploadAll(gpu_, &shaderResourceHeap_, assets_);
}

void RenderSystem::CollectRenderRequests() {
	for (GameObject* gameObject : scene_->GetGameObjects()) {
		if (!gameObject->IsActive()) {
			continue;
		}

		for (Component* component : gameObject->GetComponents()) {
			if (!component->IsEnabled()) {
				continue;
			}

			// 描きたいと宣言したコンポーネントにだけ頼む。
			IRenderable* renderable = dynamic_cast<IRenderable*>(component);
			if (renderable != nullptr) {
				renderable->SubmitRender(&renderQueue_);
			}
		}
	}
}

void RenderSystem::Draw() {
	if (renderQueue_.HasCamera()) {
		RenderContext context = {};
		context.commandList = gpu_->GetCommandList();
		// ウィンドウではなく描き先の大きさを使うので、最小化の間も0で割らない。
		context.targetWidth = static_cast<float>(gpu_->GetRenderTargetWidth());
		context.targetHeight = static_cast<float>(gpu_->GetRenderTargetHeight());
		context.deltaTime = time_->GetDeltaTime();

		// カメラの行列は、どの工程も同じものを使うので、ここで1回だけ作る。
		const CameraData& camera = renderQueue_.GetCamera();
		context.view = camera.view;
		context.projection = MakePerspectiveFovMatrix(camera.fovYRadians, context.targetWidth / context.targetHeight, camera.nearZ, camera.farZ);
		context.viewProjection = context.view * context.projection;

		passList_.Execute(context);
	}

	// カメラがないのは作る人の間違いなので、止めずにログを1回だけ出す。
	if (!renderQueue_.HasCamera() && !hasWarnedNoCamera_) {
		printf("カメラがないので、シーンを描きません。\n");
		hasWarnedNoCamera_ = true;
	}

	// 光がないのも作る人の間違いなので、止めずにログを1回だけ出す。
	if (!renderQueue_.HasDirectionalLight() && !hasWarnedNoLight_) {
		printf("平行光源がないので、環境光だけで描きます。\n");
		hasWarnedNoLight_ = true;
	}

	// 描かなかったフレームでも依頼が溜まらないように、必ず空にする。
	renderQueue_.Clear();
}

} // namespace KujataEngine
