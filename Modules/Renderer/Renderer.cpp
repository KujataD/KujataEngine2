#include "Renderer.h"

#include "Core/Assert.h"

#include <cstdio>
#include <filesystem>

using Microsoft::WRL::ComPtr;

namespace KujataEngine {

// 頂点のデータで、シェーダーのVertexInputと並びを合わせる。
struct Vertex {
	float x;
	float y;
	float z;
};

static const Vertex kTriangleVertices[3] = {
	{ 0.0f, 0.5f, 0.0f },
	{ 0.5f, -0.5f, 0.0f },
	{ -0.5f, -0.5f, 0.0f },
};

/// <summary>
/// exeの場所から、シェーダーのファイルがあるフォルダを求める。
/// </summary>
static std::filesystem::path GetShaderDirectory() {
	wchar_t exePath[MAX_PATH] = {};
	GetModuleFileNameW(nullptr, exePath, MAX_PATH);

	// exeはbuild\bin\構成名にあるので、3つ上がリポジトリの一番上になる。
	const std::filesystem::path repositoryRoot = std::filesystem::path(exePath).parent_path().parent_path().parent_path().parent_path();
	return repositoryRoot / L"Modules" / L"Renderer" / L"Shaders";
}

void Renderer::Startup(ServiceRegistry& services, FrameLoop& frameLoop) {
	// Startupは追加した順に呼ばれるので、WindowとGpuはRendererより先にAddしておく。
	gpu_ = static_cast<IGpu*>(services.Get("Gpu"));
	KUJATA_ASSERT(gpu_ != nullptr);

	window_ = static_cast<IWindow*>(services.Get("Window"));
	KUJATA_ASSERT(window_ != nullptr);

	shaderCompiler_.Initialize();

	const std::filesystem::path shaderDirectory = GetShaderDirectory();
	ComPtr<IDxcBlob> vertexShader = shaderCompiler_.Compile((shaderDirectory / L"Triangle.VS.hlsl").wstring(), L"vs_6_0");
	ComPtr<IDxcBlob> pixelShader = shaderCompiler_.Compile((shaderDirectory / L"Triangle.PS.hlsl").wstring(), L"ps_6_0");

	// シェーダーの書き間違いは普通に起きるので、止めずに描かないだけにする。
	if (vertexShader == nullptr || pixelShader == nullptr) {
		printf("シェーダーを準備できなかったので、三角形を描きません。\n");
		return;
	}

	ID3D12Device* device = gpu_->GetDevice();
	pipelineState_.Initialize(device, vertexShader.Get(), pixelShader.Get(), gpu_->GetRenderTargetFormat());
	vertexBuffer_.Initialize(device, kTriangleVertices, sizeof(Vertex), 3);

	frameLoop.Register(kRender, [this]() {
		Draw();
	});
}

void Renderer::Draw() {
	ID3D12GraphicsCommandList* list = gpu_->GetCommandList();

	// 描く範囲の設定で、忘れてもエラーにならず何も映らないだけになる。
	const float width = static_cast<float>(window_->GetClientWidth());
	const float height = static_cast<float>(window_->GetClientHeight());
	const D3D12_VIEWPORT viewport = { 0.0f, 0.0f, width, height, 0.0f, 1.0f };
	const D3D12_RECT scissor = { 0, 0, window_->GetClientWidth(), window_->GetClientHeight() };
	list->RSSetViewports(1, &viewport);
	list->RSSetScissorRects(1, &scissor);

	pipelineState_.Bind(list);
	vertexBuffer_.Bind(list);
	list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	list->DrawInstanced(3, 1, 0, 0);
}

} // namespace KujataEngine
