#pragma once

#include "Core/IModule.h"
#include "Framework/Scene.h"
#include "GpuAssetTable.h"
#include "IRenderPipeline.h"
#include "Interface/IGpu.h"
#include "Interface/IWindow.h"
#include "OpaquePass.h"
#include "PointCloudPass.h"
#include "RenderQueue.h"
#include "ShaderCompiler.h"
#include "ShaderResourceHeap.h"
#include "StandardRenderPipeline.h"

namespace KujataEngine {

/// <summary>
/// Sceneから描画の依頼を集めて、描画の手順を毎フレーム実行するモジュール。
/// </summary>
class Renderer : public IModule {
public:
	std::vector<std::string> GetRequiredServices() override;
	void Startup(ServiceRegistry& services, FrameLoop& frameLoop) override;
	void Prepare() override;

	/// <summary>
	/// 描画の手順を差し替え、手順の持ち主は呼び出し側のままにする。
	/// </summary>
	void SetRenderPipeline(IRenderPipeline* renderPipeline);

	/// <summary>
	/// 差し替えた手順からも使えるように、不透明の工程を返す。
	/// </summary>
	OpaquePass* GetOpaquePass();

private:
	/// <summary>
	/// 有効なコンポーネントから、このフレームの描画の依頼を集める。
	/// </summary>
	void CollectRenderRequests();

	void Draw();

	IGpu* gpu_ = nullptr;
	IWindow* window_ = nullptr;
	Scene* scene_ = nullptr;
	IAssetLibrary* assets_ = nullptr;

	ShaderCompiler shaderCompiler_;
	ShaderResourceHeap shaderResourceHeap_;
	RenderQueue renderQueue_;
	GpuAssetTable gpuAssets_;

	OpaquePass opaquePass_;
	PointCloudPass pointCloudPass_;
	StandardRenderPipeline standardRenderPipeline_;
	IRenderPipeline* renderPipeline_ = &standardRenderPipeline_;

	bool hasWarnedNoCamera_ = false;
	bool hasWarnedNoLight_ = false;
};

} // namespace KujataEngine
