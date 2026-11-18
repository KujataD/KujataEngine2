#include "ParticleDrawPass.h"

#include "Core/Assert.h"
#include "Core/Math/Matrix4x4.h"
#include "GraphicsKit/RootSignature.h"
#include "ParticleRendering.h"

#include <algorithm>
#include <cstdio>

using Microsoft::WRL::ComPtr;

namespace KujataEngine {

// ParticleDraw.VS.hlslとParticleDraw.PS.hlslのDrawConstantsと並びを合わせ、HLSLの16バイトの区切りに合うように詰め物を入れる。
struct DrawConstants {
	Matrix4x4 view;
	Matrix4x4 projection;
	float pointSize;
	float intensity;
	uint32_t targetCount;
	float scatteredBrightness;
};
static_assert(sizeof(DrawConstants) == 144, "DrawConstantsの大きさが変わったらParticleDraw.VS.hlslとParticleDraw.PS.hlslも直す。");

void ParticleDrawPass::Initialize(IGpu* gpu, ShaderCompiler* shaderCompiler, ShaderResourceHeap* shaderResourceHeap, const std::filesystem::path& resourcesDirectory, const std::vector<ParticleRequest>* requests, const ParticleShapeTable* shapes, const ParticleInstances* instances) {
	shaderResourceHeap_ = shaderResourceHeap;
	requests_ = requests;
	shapes_ = shapes;
	instances_ = instances;

	const std::filesystem::path shaderDirectory = resourcesDirectory / L"Shaders";
	ComPtr<IDxcBlob> vertexShader = shaderCompiler->Compile((shaderDirectory / L"ParticleDraw.VS.hlsl").wstring(), L"vs_6_0");
	ComPtr<IDxcBlob> pixelShader = shaderCompiler->Compile((shaderDirectory / L"ParticleDraw.PS.hlsl").wstring(), L"ps_6_0");

	// シェーダーの書き間違いは普通に起きるので、止めずに描かないだけにする。
	if (vertexShader == nullptr || pixelShader == nullptr) {
		printf("粒を描くシェーダーを準備できなかったので、粒子を描きません。\n");
		return;
	}

	CreateRootSignature(gpu->GetDevice());
	CreatePipelineState(gpu, vertexShader.Get(), pixelShader.Get());
	constantBuffer_.Initialize(gpu->GetDevice(), sizeof(DrawConstants), kMaxParticleRequestCount);
	isReady_ = true;
}

void ParticleDrawPass::Execute(const RenderContext& context) {
	if (!isReady_ || requests_->empty()) {
		return;
	}

	ID3D12GraphicsCommandList* list = context.commandList;

	// 工程ごとに独立させるため、前の工程と同じ範囲をここでも設定する。設定は粒子ごとに変わらないので1回だけ書く。
	const D3D12_VIEWPORT viewport = { 0.0f, 0.0f, context.targetWidth, context.targetHeight, 0.0f, 1.0f };
	const D3D12_RECT scissor = { 0, 0, static_cast<LONG>(context.targetWidth), static_cast<LONG>(context.targetHeight) };
	list->RSSetViewports(1, &viewport);
	list->RSSetScissorRects(1, &scissor);
	list->SetPipelineState(pipelineState_.Get());
	list->SetGraphicsRootSignature(rootSignature_.Get());
	shaderResourceHeap_->Bind(list);
	list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	for (UINT i = 0; i < requests_->size(); i++) {
		const ParticleRequest& request = (*requests_)[i];

		// 動かす工程が準備できずに粒のバッファがないときは、描く物もない。
		const ParticleInstance* instance = instances_->Find(request.instanceId);
		if (instance == nullptr) {
			continue;
		}
		const GpuParticleShape& shape = shapes_->Get(request.shape);

		DrawConstants constants = {};
		constants.view = context.view;
		constants.projection = context.projection;
		constants.pointSize = request.pointSize;
		constants.intensity = request.intensity;
		constants.targetCount = shape.targetCount;
		constants.scatteredBrightness = request.scatteredBrightness;
		constantBuffer_.Write(i, &constants);

		constantBuffer_.Bind(list, kConstantsRootParameterIndex, i);
		list->SetGraphicsRootDescriptorTable(kStatesRootParameterIndex, shaderResourceHeap_->GetGpuHandle(instance->readHeapIndex));
		list->SetGraphicsRootDescriptorTable(kTargetsRootParameterIndex, shape.targetsGpuHandle);

		// 粒は面にランダムに並んでいるので、頭から割合の分だけ描けば全体が同じようにまばらになる。
		const float density = std::clamp(request.density, 0.0f, 1.0f);
		const UINT drawnPointCount = static_cast<UINT>(static_cast<float>(instance->pointCount) * density);

		// 頂点4つの板を、描く粒の数だけ繰り返して描く。
		list->DrawInstanced(4, drawnPointCount, 0, 0);
	}
}

void ParticleDrawPass::CreateRootSignature(ID3D12Device* device) {
	const D3D12_DESCRIPTOR_RANGE statesRange = MakeSingleRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0);
	const D3D12_DESCRIPTOR_RANGE targetsRange = MakeSingleRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1);

	D3D12_ROOT_PARAMETER rootParameters[3] = {};

	// b0で、頂点シェーダーがカメラと板の大きさを、ピクセルシェーダーが明るさを受け取る。
	rootParameters[kConstantsRootParameterIndex].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[kConstantsRootParameterIndex].Descriptor.ShaderRegister = 0;
	rootParameters[kConstantsRootParameterIndex].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// 頂点シェーダーのt0で粒の位置を、t1で集まる先の色を受け取る。
	rootParameters[kStatesRootParameterIndex] = MakeTableParameter(&statesRange, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[kTargetsRootParameterIndex] = MakeTableParameter(&targetsRange, D3D12_SHADER_VISIBILITY_VERTEX);

	// 頂点バッファを使わないので、入力の設定を許す旗は立てない。
	D3D12_ROOT_SIGNATURE_DESC description = {};
	description.NumParameters = _countof(rootParameters);
	description.pParameters = rootParameters;
	description.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
	rootSignature_ = CreateRootSignatureFromDescription(device, description);
}

void ParticleDrawPass::CreatePipelineState(IGpu* gpu, IDxcBlob* vertexShader, IDxcBlob* pixelShader) {
	D3D12_GRAPHICS_PIPELINE_STATE_DESC description = {};
	description.pRootSignature = rootSignature_.Get();
	description.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
	description.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };

	// 頂点の入力はなく、SV_VertexIDとSV_InstanceIDだけで描く。
	description.InputLayout = { nullptr, 0 };

	// 重なった粒の明るさを足し合わせ、描く順番に左右されないようにする。
	D3D12_RENDER_TARGET_BLEND_DESC& blend = description.BlendState.RenderTarget[0];
	blend.BlendEnable = TRUE;
	blend.SrcBlend = D3D12_BLEND_ONE;
	blend.DestBlend = D3D12_BLEND_ONE;
	blend.BlendOp = D3D12_BLEND_OP_ADD;

	// 画面のアルファは変えない。
	blend.SrcBlendAlpha = D3D12_BLEND_ZERO;
	blend.DestBlendAlpha = D3D12_BLEND_ONE;
	blend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	description.SampleMask = UINT32_MAX;
	description.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;

	// 板の表と裏を気にしなくて済むように、どちら向きの三角形も描く。
	description.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	description.RasterizerState.DepthClipEnable = TRUE;
	description.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	description.NumRenderTargets = 1;
	description.RTVFormats[0] = gpu->GetRenderTargetFormat();

	// 深度を読むので不透明な物には隠れ、書かないので粒どうしは隠し合わない。
	description.DepthStencilState.DepthEnable = TRUE;
	description.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	description.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	description.DSVFormat = gpu->GetDepthFormat();
	description.SampleDesc.Count = 1;

	const HRESULT result = gpu->GetDevice()->CreateGraphicsPipelineState(&description, IID_PPV_ARGS(&pipelineState_));
	KUJATA_ASSERT(SUCCEEDED(result));
}

} // namespace KujataEngine
