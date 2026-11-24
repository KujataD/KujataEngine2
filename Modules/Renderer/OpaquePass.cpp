#include "OpaquePass.h"

#include "Core/Assert.h"
#include "Core/Math/Matrix4x4.h"
#include "GraphicsKit/RootSignature.h"
#include "Interface/AssetData.h"

#include <cstddef>
#include <cstdio>

using Microsoft::WRL::ComPtr;

namespace KujataEngine {

// Object3D.VS.hlslのTransformDataと並びを合わせる。
struct TransformData {
	Matrix4x4 worldViewProjection;
	Matrix4x4 worldInverseTranspose;
	Vector4 tint;
};
static_assert(sizeof(TransformData) == 144, "TransformDataの大きさが変わったらObject3D.VS.hlslも直す。");

// Object3D.PS.hlslのLightDataと並びを合わせ、HLSLの16バイトの区切りに合うように詰め物を入れる。
struct LightData {
	Vector3 direction;
	float intensity;
	Vector3 color;
	float padding1;
	Vector3 ambientColor;
	float padding2;
};
static_assert(sizeof(LightData) == 48, "LightDataの大きさが変わったらObject3D.PS.hlslも直す。");

// 光が当たらない面にも足す明るさ。
static const Vector3 kAmbientColor = { 0.3f, 0.3f, 0.3f };

// 1フレームに描ける物の数。
static const UINT kMaxDrawCount = 1024;

void OpaquePass::Initialize(IGpu* gpu, IRenderPasses* renderPasses, const std::filesystem::path& resourcesDirectory, const RenderQueue* renderQueue, GpuAssetTable* gpuAssets) {
	shaderResourceHeap_ = renderPasses->GetShaderResourceHeap();
	renderQueue_ = renderQueue;
	gpuAssets_ = gpuAssets;

	const std::filesystem::path shaderDirectory = resourcesDirectory / L"Shaders";
	ShaderCompiler* shaderCompiler = renderPasses->GetShaderCompiler();
	ComPtr<IDxcBlob> vertexShader = shaderCompiler->Compile((shaderDirectory / L"Object3D.VS.hlsl").wstring(), L"vs_6_0");
	ComPtr<IDxcBlob> pixelShader = shaderCompiler->Compile((shaderDirectory / L"Object3D.PS.hlsl").wstring(), L"ps_6_0");

	// シェーダーの書き間違いは普通に起きるので、止めずに描かないだけにする。
	if (vertexShader == nullptr || pixelShader == nullptr) {
		printf("シェーダーを準備できなかったので、不透明な物を描きません。\n");
		return;
	}

	ID3D12Device* device = gpu->GetDevice();
	CreateRootSignature(device);
	CreatePipelineState(gpu, vertexShader.Get(), pixelShader.Get());
	transformBuffer_.Initialize(device, sizeof(TransformData), kMaxDrawCount);
	lightBuffer_.Initialize(device, sizeof(LightData), 1);
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

	list->SetPipelineState(pipelineState_.Get());
	list->SetGraphicsRootSignature(rootSignature_.Get());
	shaderResourceHeap_->Bind(list);
	list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 光がないフレームは強さを0のまま書くので、環境光だけで描かれる。
	LightData lightData = {};
	if (renderQueue_->HasDirectionalLight()) {
		const DirectionalLightData& light = renderQueue_->GetDirectionalLight();
		lightData.direction = light.direction;
		lightData.color = light.color;
		lightData.intensity = light.intensity;
	}
	lightData.ambientColor = kAmbientColor;
	lightBuffer_.Write(0, &lightData);
	lightBuffer_.Bind(list, kLightRootParameterIndex, 0);

	const std::vector<DrawRequest>& drawRequests = renderQueue_->GetDrawRequests();
	// 物の数はゲームしだいで増えるので、止めずに上限までだけを描く。
	if (drawRequests.size() > kMaxDrawCount && !hasWarnedTooManyDraws_) {
		printf("描く物が%u個を超えたので、超えた分は描きません。\n", kMaxDrawCount);
		hasWarnedTooManyDraws_ = true;
	}

	for (UINT drawIndex = 0; drawIndex < drawRequests.size() && drawIndex < kMaxDrawCount; drawIndex++) {
		const DrawRequest& request = drawRequests[drawIndex];

		TransformData transformData = {};
		transformData.worldViewProjection = request.world * context.viewProjection;
		transformData.worldInverseTranspose = Transpose(Inverse(request.world));
		transformData.tint = request.tint;
		transformBuffer_.Write(drawIndex, &transformData);
		transformBuffer_.Bind(list, kTransformRootParameterIndex, drawIndex);

		GpuMesh& mesh = gpuAssets_->GetMesh(request.mesh);
		mesh.vertexBuffer.Bind(list);
		mesh.indexBuffer.Bind(list);
		gpuAssets_->GetTexture(request.texture).Bind(list, kTextureRootParameterIndex);

		list->DrawIndexedInstanced(mesh.indexBuffer.GetIndexCount(), 1, 0, 0, 0);
	}
}

void OpaquePass::CreateRootSignature(ID3D12Device* device) {
	const D3D12_DESCRIPTOR_RANGE textureRange = MakeSingleRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0);

	D3D12_ROOT_PARAMETER rootParameters[3] = {};

	// 頂点シェーダーのb0で行列を、ピクセルシェーダーのt0でテクスチャを、b1で光を受け取る。
	rootParameters[kTransformRootParameterIndex] = MakeConstantsParameter(0, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[kTextureRootParameterIndex] = MakeTableParameter(&textureRange, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[kLightRootParameterIndex] = MakeConstantsParameter(1, D3D12_SHADER_VISIBILITY_PIXEL);

	// s0はミップの間も補間するトライリニアで読み、0から1の外は繰り返す。
	const D3D12_STATIC_SAMPLER_DESC sampler = MakeWrapLinearSampler(0, D3D12_SHADER_VISIBILITY_PIXEL);

	D3D12_ROOT_SIGNATURE_DESC description = {};
	description.NumParameters = _countof(rootParameters);
	description.pParameters = rootParameters;
	description.NumStaticSamplers = 1;
	description.pStaticSamplers = &sampler;
	description.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignature_ = CreateRootSignatureFromDescription(device, description);
}

void OpaquePass::CreatePipelineState(IGpu* gpu, IDxcBlob* vertexShader, IDxcBlob* pixelShader) {
	// AssetData.hのMeshVertexとObject3D.VS.hlslのVertexInputと並びを合わせる。
	static_assert(sizeof(MeshVertex) == 32, "MeshVertexの大きさが変わったら入力の設定も直す。");

	D3D12_INPUT_ELEMENT_DESC inputElements[3] = {};
	inputElements[0].SemanticName = "POSITION";
	inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElements[0].AlignedByteOffset = offsetof(MeshVertex, position);
	inputElements[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	inputElements[1].SemanticName = "TEXCOORD";
	inputElements[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElements[1].AlignedByteOffset = offsetof(MeshVertex, uv);
	inputElements[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	inputElements[2].SemanticName = "NORMAL";
	inputElements[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElements[2].AlignedByteOffset = offsetof(MeshVertex, normal);
	inputElements[2].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC description = {};
	description.pRootSignature = rootSignature_.Get();
	description.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
	description.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };
	description.InputLayout = { inputElements, _countof(inputElements) };
	description.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	description.SampleMask = UINT32_MAX;
	description.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	description.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	description.RasterizerState.DepthClipEnable = TRUE;
	description.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	description.NumRenderTargets = 1;
	description.RTVFormats[0] = gpu->GetRenderTargetFormat();
	description.DepthStencilState.DepthEnable = TRUE;
	description.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	description.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	description.DSVFormat = gpu->GetDepthFormat();
	description.SampleDesc.Count = 1;

	const HRESULT result = gpu->GetDevice()->CreateGraphicsPipelineState(&description, IID_PPV_ARGS(&pipelineState_));
	KUJATA_ASSERT(SUCCEEDED(result));
}

} // namespace KujataEngine
