#include "OpaquePass.h"

#include "Core/Assert.h"
#include "Core/Math/MathUtil.h"

#include <cstdio>

using Microsoft::WRL::ComPtr;

namespace KujataEngine {

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

	ID3D12Device* device = gpu->GetDevice();
	pipelineState_.Initialize(device, vertexShader.Get(), pixelShader.Get(), gpu->GetRenderTargetFormat(), gpu->GetDepthFormat());
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
	list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	const CameraData& camera = context.renderQueue->GetCamera();
	const float aspectRatio = context.targetWidth / context.targetHeight;
	const Matrix4x4 projection = MakePerspectiveFovMatrix(camera.fovYRadians, aspectRatio, camera.nearZ, camera.farZ);
	const Matrix4x4 viewProjection = camera.view * projection;

	const std::vector<DrawRequest>& drawRequests = context.renderQueue->GetDrawRequests();
	KUJATA_ASSERT(drawRequests.size() <= kMaxDrawCount);

	for (UINT drawIndex = 0; drawIndex < drawRequests.size() && drawIndex < kMaxDrawCount; drawIndex++) {
		const DrawRequest& request = drawRequests[drawIndex];

		TransformData transformData = {};
		transformData.worldViewProjection = request.world * viewProjection;
		transformBuffer_.Write(drawIndex, &transformData);
		transformBuffer_.Bind(list, PipelineState::kTransformRootParameterIndex, drawIndex);

		GpuMesh& mesh = context.gpuAssets->GetMesh(request.mesh);
		mesh.vertexBuffer.Bind(list);
		mesh.indexBuffer.Bind(list);
		context.gpuAssets->GetTexture(request.texture).Bind(list, PipelineState::kTextureRootParameterIndex);

		list->DrawIndexedInstanced(mesh.indexBuffer.GetIndexCount(), 1, 0, 0, 0);
	}
}

} // namespace KujataEngine
