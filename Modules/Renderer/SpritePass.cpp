#include "SpritePass.h"

#include "Core/Assert.h"
#include "Core/Math/Matrix4x4.h"
#include "GraphicsKit/RootSignature.h"
#include "Interface/AssetData.h"

#include <algorithm>
#include <cstddef>
#include <cstdio>

using Microsoft::WRL::ComPtr;

namespace KujataEngine {

// Sprite.VS.hlslのTransformDataと並びを合わせる。
struct SpriteTransformData {
	Matrix4x4 worldViewProjection;
};
static_assert(sizeof(SpriteTransformData) == 64, "SpriteTransformDataの大きさが変わったらSprite.VS.hlslも直す。");

// Sprite.PS.hlslのMaterialDataと並びを合わせる。
struct SpriteMaterialData {
	Vector4 color;
	Matrix4x4 uvTransform;
};
static_assert(sizeof(SpriteMaterialData) == 80, "SpriteMaterialDataの大きさが変わったらSprite.PS.hlslも直す。");

// 1フレームに描けるスプライトの数。
static const UINT kMaxDrawCount = 1024;

void SpritePass::Initialize(IGpu* gpu, IRenderPasses* renderPasses, const std::filesystem::path& resourcesDirectory, const RenderQueue* renderQueue, GpuAssetTable* gpuAssets) {
	shaderResourceHeap_ = renderPasses->GetShaderResourceHeap();
	renderQueue_ = renderQueue;
	gpuAssets_ = gpuAssets;

	const std::filesystem::path shaderDirectory = resourcesDirectory / L"Shaders";
	ShaderCompiler* shaderCompiler = renderPasses->GetShaderCompiler();
	ComPtr<IDxcBlob> vertexShader = shaderCompiler->Compile((shaderDirectory / L"Sprite.VS.hlsl").wstring(), L"vs_6_0");
	ComPtr<IDxcBlob> pixelShader = shaderCompiler->Compile((shaderDirectory / L"Sprite.PS.hlsl").wstring(), L"ps_6_0");

	// シェーダーの書き間違いは普通に起きるので、止めずに描かないだけにする。
	if (vertexShader == nullptr || pixelShader == nullptr) {
		printf("シェーダーを準備できなかったので、スプライトを描きません。\n");
		return;
	}

	ID3D12Device* device = gpu->GetDevice();
	CreateRootSignature(device);
	CreatePipelineState(gpu, vertexShader.Get(), pixelShader.Get());
	transformBuffer_.Initialize(device, sizeof(SpriteTransformData), kMaxDrawCount);
	materialBuffer_.Initialize(device, sizeof(SpriteMaterialData), kMaxDrawCount);
	isReady_ = true;
}

void SpritePass::Execute(const RenderContext& context) {
	if (!isReady_) {
		return;
	}

	// 深度を書かないので、前後は描く順で決まる。同じ番号は頼んだ順を保つ。
	std::vector<SpriteRequest> sprites = renderQueue_->GetSpriteRequests();
	if (sprites.empty()) {
		return;
	}
	std::stable_sort(sprites.begin(), sprites.end(), [](const SpriteRequest& a, const SpriteRequest& b) {
		return a.sortingOrder < b.sortingOrder;
	});

	ID3D12GraphicsCommandList* list = context.commandList;

	// 描く範囲の設定で、忘れてもエラーにならず何も映らないだけになる。
	const D3D12_VIEWPORT viewport = {0.0f, 0.0f, context.targetWidth, context.targetHeight, 0.0f, 1.0f};
	const D3D12_RECT scissor = {0, 0, static_cast<LONG>(context.targetWidth), static_cast<LONG>(context.targetHeight)};
	list->RSSetViewports(1, &viewport);
	list->RSSetScissorRects(1, &scissor);

	list->SetPipelineState(pipelineState_.Get());
	list->SetGraphicsRootSignature(rootSignature_.Get());
	shaderResourceHeap_->Bind(list);
	list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// スプライトの数はゲームしだいで増えるので、止めずに上限までだけを描く。
	if (sprites.size() > kMaxDrawCount && !hasWarnedTooManyDraws_) {
		printf("スプライトが%u枚を超えたので、超えた分は描きません。\n", kMaxDrawCount);
		hasWarnedTooManyDraws_ = true;
	}

	for (UINT drawIndex = 0; drawIndex < sprites.size() && drawIndex < kMaxDrawCount; drawIndex++) {
		const SpriteRequest& request = sprites[drawIndex];

		SpriteTransformData transformData = {};
		transformData.worldViewProjection = request.world * context.viewProjection;
		transformBuffer_.Write(drawIndex, &transformData);
		transformBuffer_.Bind(list, kTransformRootParameterIndex, drawIndex);

		SpriteMaterialData materialData = {};
		materialData.color = request.color;
		materialData.uvTransform = request.uvTransform;
		materialBuffer_.Write(drawIndex, &materialData);
		materialBuffer_.Bind(list, kMaterialRootParameterIndex, drawIndex);

		GpuMesh& mesh = gpuAssets_->GetMesh(request.quad);
		mesh.vertexBuffer.Bind(list);
		mesh.indexBuffer.Bind(list);
		gpuAssets_->GetTexture(request.texture).Bind(list, kTextureRootParameterIndex);

		list->DrawIndexedInstanced(mesh.indexBuffer.GetIndexCount(), 1, 0, 0, 0);
	}
}

void SpritePass::CreateRootSignature(ID3D12Device* device) {
	const D3D12_DESCRIPTOR_RANGE textureRange = MakeSingleRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0);

	D3D12_ROOT_PARAMETER rootParameters[3] = {};

	// どちらもb0だが、行列は頂点シェーダーだけ、色とUVはピクセルシェーダーだけに見せるので重ならない。
	rootParameters[kTransformRootParameterIndex] = MakeConstantsParameter(0, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[kTextureRootParameterIndex] = MakeTableParameter(&textureRange, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[kMaterialRootParameterIndex] = MakeConstantsParameter(0, D3D12_SHADER_VISIBILITY_PIXEL);

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

void SpritePass::CreatePipelineState(IGpu* gpu, IDxcBlob* vertexShader, IDxcBlob* pixelShader) {
	// AssetData.hのMeshVertexとSprite.VS.hlslのVertexInputと並びを合わせる。
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
	description.VS = {vertexShader->GetBufferPointer(), vertexShader->GetBufferSize()};
	description.PS = {pixelShader->GetBufferPointer(), pixelShader->GetBufferSize()};
	description.InputLayout = {inputElements, _countof(inputElements)};

	// 画像のアルファの分だけ、後ろの色を残して混ぜる。
	D3D12_RENDER_TARGET_BLEND_DESC& blend = description.BlendState.RenderTarget[0];
	blend.BlendEnable = TRUE;
	blend.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blend.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blend.BlendOp = D3D12_BLEND_OP_ADD;
	blend.SrcBlendAlpha = D3D12_BLEND_ONE;
	blend.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
	blend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	description.SampleMask = UINT32_MAX;

	// 裏返しても見えるように、両面を描く。
	description.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	description.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	description.RasterizerState.DepthClipEnable = TRUE;
	description.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	description.NumRenderTargets = 1;
	description.RTVFormats[0] = gpu->GetRenderTargetFormat();

	// 不透明な物には隠れるが、スプライトどうしは深度で争わないように、比べるだけで書かない。
	description.DepthStencilState.DepthEnable = TRUE;
	description.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	description.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	description.DSVFormat = gpu->GetDepthFormat();
	description.SampleDesc.Count = 1;

	const HRESULT result = gpu->GetDevice()->CreateGraphicsPipelineState(&description, IID_PPV_ARGS(&pipelineState_));
	KUJATA_ASSERT(SUCCEEDED(result));
}

} // namespace KujataEngine
