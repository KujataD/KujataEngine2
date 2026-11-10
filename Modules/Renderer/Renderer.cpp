#include "Renderer.h"

#include "Core/Assert.h"
#include "Interface/IRenderable.h"

#include <Windows.h>
#include <cstdio>
#include <filesystem>

namespace KujataEngine {

// テクスチャの棚に置ける数。
static const UINT kShaderResourceHeapCapacity = 64;

/// <summary>
/// exeの場所から、Rendererのデータがあるフォルダを求める。
/// </summary>
static std::filesystem::path GetRendererDataDirectory() {
	wchar_t exePath[MAX_PATH] = {};
	GetModuleFileNameW(nullptr, exePath, MAX_PATH);

	const std::filesystem::path exeDirectory = std::filesystem::path(exePath).parent_path();

	// 配布したexeは、隣のDataを使う。
	const std::filesystem::path packagedDirectory = exeDirectory / L"Data" / L"Renderer";
	if (std::filesystem::exists(packagedDirectory)) {
		return packagedDirectory;
	}

	// 開発中のexeはbuild\bin\構成名にあるので、3つ上がリポジトリの一番上になる。
	const std::filesystem::path repositoryRoot = exeDirectory.parent_path().parent_path().parent_path();
	return repositoryRoot / L"Modules" / L"Renderer";
}

std::vector<std::string> Renderer::GetRequiredServices() {
	return { kGpuServiceName, kWindowServiceName, kSceneServiceName };
}

void Renderer::Startup(ServiceRegistry& services, FrameLoop& frameLoop) {
	gpu_ = static_cast<IGpu*>(services.Get(kGpuServiceName));
	KUJATA_ASSERT(gpu_ != nullptr);

	window_ = static_cast<IWindow*>(services.Get(kWindowServiceName));
	KUJATA_ASSERT(window_ != nullptr);

	scene_ = static_cast<Scene*>(services.Get(kSceneServiceName));
	KUJATA_ASSERT(scene_ != nullptr);

	// DirectXTexが画像を読むのにCOMを使う。
	const HRESULT comResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	KUJATA_ASSERT(SUCCEEDED(comResult));

	shaderCompiler_.Initialize();
	shaderResourceHeap_.Initialize(gpu_->GetDevice(), kShaderResourceHeapCapacity);

	opaquePass_.Initialize(gpu_, &shaderCompiler_, &shaderResourceHeap_, GetRendererDataDirectory());
	standardRenderPipeline_.AddPass(&opaquePass_);

	frameLoop.Register(kSubmitRender, [this]() {
		CollectRenderRequests();
	});
	frameLoop.Register(kRender, [this]() {
		Draw();
	});
}

void Renderer::Shutdown() {
	CoUninitialize();
}

void Renderer::SetRenderPipeline(IRenderPipeline* renderPipeline) {
	KUJATA_ASSERT(renderPipeline != nullptr);

	renderPipeline_ = renderPipeline;
}

OpaquePass* Renderer::GetOpaquePass() {
	return &opaquePass_;
}

void Renderer::CollectRenderRequests() {
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

void Renderer::Draw() {
	if (renderQueue_.HasCamera()) {
		RenderContext context = {};
		context.commandList = gpu_->GetCommandList();
		context.renderQueue = &renderQueue_;
		context.targetWidth = static_cast<float>(window_->GetClientWidth());
		context.targetHeight = static_cast<float>(window_->GetClientHeight());
		renderPipeline_->Render(context);
	}

	// カメラがないのは作る人の間違いなので、止めずに毎フレームは出さない。
	if (!renderQueue_.HasCamera() && !hasWarnedNoCamera_) {
		printf("カメラがないので、シーンを描きません。\n");
		hasWarnedNoCamera_ = true;
	}

	// 描かなかったフレームでも依頼が溜まらないように、必ず空にする。
	renderQueue_.Clear();
}

} // namespace KujataEngine
