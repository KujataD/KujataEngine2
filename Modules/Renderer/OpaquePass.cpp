#include "OpaquePass.h"

#include "Core/Assert.h"
#include "Core/Math/MathUtil.h"

#include <cstdio>

using Microsoft::WRL::ComPtr;

namespace KujataEngine {

// 頂点のデータで、シェーダーのVertexInputと並びを合わせる。
struct Vertex {
	float x;
	float y;
	float z;
	float u;
	float v;
};

// 立方体の頂点で、面ごとにUVが違うので1面に4つずつ持つ。
static const Vertex kCubeVertices[24] = {
	{ -0.5f, -0.5f, -0.5f, 0.0f, 1.0f },
	{ -0.5f, 0.5f, -0.5f, 0.0f, 0.0f },
	{ 0.5f, 0.5f, -0.5f, 1.0f, 0.0f },
	{ 0.5f, -0.5f, -0.5f, 1.0f, 1.0f },
	{ 0.5f, -0.5f, 0.5f, 0.0f, 1.0f },
	{ 0.5f, 0.5f, 0.5f, 0.0f, 0.0f },
	{ -0.5f, 0.5f, 0.5f, 1.0f, 0.0f },
	{ -0.5f, -0.5f, 0.5f, 1.0f, 1.0f },
	{ -0.5f, -0.5f, 0.5f, 0.0f, 1.0f },
	{ -0.5f, 0.5f, 0.5f, 0.0f, 0.0f },
	{ -0.5f, 0.5f, -0.5f, 1.0f, 0.0f },
	{ -0.5f, -0.5f, -0.5f, 1.0f, 1.0f },
	{ 0.5f, -0.5f, -0.5f, 0.0f, 1.0f },
	{ 0.5f, 0.5f, -0.5f, 0.0f, 0.0f },
	{ 0.5f, 0.5f, 0.5f, 1.0f, 0.0f },
	{ 0.5f, -0.5f, 0.5f, 1.0f, 1.0f },
	{ -0.5f, 0.5f, -0.5f, 0.0f, 1.0f },
	{ -0.5f, 0.5f, 0.5f, 0.0f, 0.0f },
	{ 0.5f, 0.5f, 0.5f, 1.0f, 0.0f },
	{ 0.5f, 0.5f, -0.5f, 1.0f, 1.0f },
	{ 0.5f, -0.5f, -0.5f, 0.0f, 1.0f },
	{ 0.5f, -0.5f, 0.5f, 0.0f, 0.0f },
	{ -0.5f, -0.5f, 0.5f, 1.0f, 0.0f },
	{ -0.5f, -0.5f, -0.5f, 1.0f, 1.0f },
};

// 外から見て時計回りが表になるように、三角形12枚の頂点の番号を並べる。
static const uint16_t kCubeIndices[36] = {
	0, 1, 2, 0, 2, 3,
	4, 5, 6, 4, 6, 7,
	8, 9, 10, 8, 10, 11,
	12, 13, 14, 12, 14, 15,
	16, 17, 18, 16, 18, 19,
	20, 21, 22, 20, 22, 23,
};

// シェーダーのTransformDataと並びを合わせる。
struct TransformData {
	Matrix4x4 worldViewProjection;
};

// 1フレームに描ける物の数。
static const UINT kMaxDrawCount = 1024;

void OpaquePass::Initialize(IGpu* gpu, ShaderCompiler* shaderCompiler, ShaderResourceHeap* shaderResourceHeap, const std::filesystem::path& dataDirectory) {
	shaderResourceHeap_ = shaderResourceHeap;

	const std::filesystem::path shaderDirectory = dataDirectory / L"Shaders";
	ComPtr<IDxcBlob> vertexShader = shaderCompiler->Compile((shaderDirectory / L"Object3D.VS.hlsl").wstring(), L"vs_6_0");
	ComPtr<IDxcBlob> pixelShader = shaderCompiler->Compile((shaderDirectory / L"Object3D.PS.hlsl").wstring(), L"ps_6_0");

	// シェーダーの書き間違いは普通に起きるので、止めずに描かないだけにする。
	if (vertexShader == nullptr || pixelShader == nullptr) {
		printf("シェーダーを準備できなかったので、不透明な物を描きません。\n");
		return;
	}

	// 画像がないのは作る人の間違いなので、止めずに描かないだけにする。
	if (!texture_.Load(gpu, shaderResourceHeap, (dataDirectory / L"Textures" / L"uvChecker.png").wstring())) {
		printf("テクスチャを準備できなかったので、不透明な物を描きません。\n");
		return;
	}

	ID3D12Device* device = gpu->GetDevice();
	pipelineState_.Initialize(device, vertexShader.Get(), pixelShader.Get(), gpu->GetRenderTargetFormat(), gpu->GetDepthFormat());
	vertexBuffer_.Initialize(device, kCubeVertices, sizeof(Vertex), 24);
	indexBuffer_.Initialize(device, kCubeIndices, 36);
	transformBuffer_.Initialize(device, sizeof(TransformData), kMaxDrawCount);
	isReady_ = true;
}

void OpaquePass::Execute(const RenderContext& context) {
	if (!isReady_) {
		return;
	}

	ID3D12GraphicsCommandList* list = context.commandList;

	// 描く範囲の設定で、忘れてもエラーにならず何も映らないだけになる。
	const D3D12_VIEWPORT viewport = { 0.0f, 0.0f, context.targetWidth, context.targetHeight, 0.0f, 1.0f };
	const D3D12_RECT scissor = { 0, 0, static_cast<LONG>(context.targetWidth), static_cast<LONG>(context.targetHeight) };
	list->RSSetViewports(1, &viewport);
	list->RSSetScissorRects(1, &scissor);

	pipelineState_.Bind(list);
	shaderResourceHeap_->Bind(list);
	vertexBuffer_.Bind(list);
	indexBuffer_.Bind(list);
	list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	texture_.Bind(list, PipelineState::kTextureRootParameterIndex);

	const CameraData& camera = context.renderQueue->GetCamera();
	const float aspectRatio = context.targetWidth / context.targetHeight;
	const Matrix4x4 projection = MakePerspectiveFovMatrix(camera.fovYRadians, aspectRatio, camera.nearZ, camera.farZ);
	const Matrix4x4 viewProjection = camera.view * projection;

	const std::vector<Matrix4x4>& cubeWorlds = context.renderQueue->GetCubeWorlds();
	KUJATA_ASSERT(cubeWorlds.size() <= kMaxDrawCount);

	for (UINT drawIndex = 0; drawIndex < cubeWorlds.size() && drawIndex < kMaxDrawCount; drawIndex++) {
		TransformData transformData = {};
		transformData.worldViewProjection = cubeWorlds[drawIndex] * viewProjection;
		transformBuffer_.Write(drawIndex, &transformData);

		transformBuffer_.Bind(list, PipelineState::kTransformRootParameterIndex, drawIndex);
		list->DrawIndexedInstanced(indexBuffer_.GetIndexCount(), 1, 0, 0, 0);
	}
}

} // namespace KujataEngine
