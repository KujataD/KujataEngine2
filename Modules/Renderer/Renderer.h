#pragma once

#include "ConstantBuffer.h"
#include "Core/IModule.h"
#include "IndexBuffer.h"
#include "Interface/IGpu.h"
#include "Interface/IWindow.h"
#include "PipelineState.h"
#include "ShaderCompiler.h"
#include "ShaderResourceHeap.h"
#include "Texture.h"
#include "VertexBuffer.h"

namespace KujataEngine {

/// <summary>
/// 何を描くかを決めて、毎フレーム描画の命令を書くモジュール。
/// </summary>
class Renderer : public IModule {
public:
	std::vector<std::string> GetRequiredServices() override;

	void Startup(ServiceRegistry& services, FrameLoop& frameLoop) override;
	void Shutdown() override;

private:
	void Update();
	void Draw();

	IGpu* gpu_ = nullptr;
	IWindow* window_ = nullptr;

	ShaderCompiler shaderCompiler_;
	PipelineState pipelineState_;
	VertexBuffer vertexBuffer_;
	IndexBuffer indexBuffer_;
	ConstantBuffer transformBuffer_;
	ShaderResourceHeap shaderResourceHeap_;
	Texture texture_;

	float rotationYRadians_ = 0.0f;
};

} // namespace KujataEngine
