#pragma once

#include "Core/IModule.h"
#include "Framework/Scene.h"
#include "GpuAssetTable.h"
#include "GraphicsKit/ShaderCompiler.h"
#include "GraphicsKit/ShaderResourceHeap.h"
#include "Interface/IGpu.h"
#include "Interface/ITime.h"
#include "Interface/IWindow.h"
#include "OpaquePass.h"
#include "PointCloudPass.h"
#include "RenderPassList.h"
#include "RenderQueue.h"

namespace KujataEngine {

/// <summary>
/// Rendererモジュールの中身で、Sceneから描画の依頼を集め、登録された工程を毎フレーム実行する。
/// </summary>
class RenderSystem {
public:
	std::vector<std::string> GetProvidedServices();
	std::vector<std::string> GetRequiredServices();
	void Startup(ServiceRegistry& services, FrameLoop& frameLoop);
	void Prepare();

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
	ITime* time_ = nullptr;

	ShaderCompiler shaderCompiler_;
	ShaderResourceHeap shaderResourceHeap_;
	RenderQueue renderQueue_;
	GpuAssetTable gpuAssets_;
	RenderPassList passList_;

	OpaquePass opaquePass_;
	PointCloudPass pointCloudPass_;

	bool hasWarnedNoCamera_ = false;
	bool hasWarnedNoLight_ = false;
};

} // namespace KujataEngine
