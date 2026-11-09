#pragma once

#include "Core/IModule.h"
#include "IRenderPipeline.h"
#include "Interface/IGpu.h"
#include "Interface/IWindow.h"
#include "OpaquePass.h"
#include "RenderQueue.h"
#include "ShaderCompiler.h"
#include "ShaderResourceHeap.h"
#include "StandardRenderPipeline.h"

namespace KujataEngine {

/// <summary>
/// Sceneから描画の依頼を受けて、描画の手順を毎フレーム実行するモジュール。
/// </summary>
class Renderer : public IModule {
public:
	std::vector<std::string> GetProvidedServices() override;
	std::vector<std::string> GetRequiredServices() override;
	void Startup(ServiceRegistry& services, FrameLoop& frameLoop) override;
	void Shutdown() override;

	/// <summary>
	/// 描画の手順を差し替え、手順の持ち主は呼び出し側のままにする。
	/// </summary>
	void SetRenderPipeline(IRenderPipeline* renderPipeline);

	/// <summary>
	/// 差し替えた手順からも使えるように、不透明の工程を返す。
	/// </summary>
	OpaquePass* GetOpaquePass();

private:
	void Draw();

	IGpu* gpu_ = nullptr;
	IWindow* window_ = nullptr;

	ShaderCompiler shaderCompiler_;
	ShaderResourceHeap shaderResourceHeap_;
	RenderQueue renderQueue_;

	OpaquePass opaquePass_;
	StandardRenderPipeline standardRenderPipeline_;
	IRenderPipeline* renderPipeline_ = &standardRenderPipeline_;

	bool hasWarnedNoCamera_ = false;
};

} // namespace KujataEngine
