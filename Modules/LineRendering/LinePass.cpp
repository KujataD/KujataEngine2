#include "LinePass.h"

#include "Core/Assert.h"
#include "GraphicsKit/RootSignature.h"
#include "GraphicsKit/UploadBuffer.h"

#include <cstddef>
#include <cstdio>
#include <cstring>

using Microsoft::WRL::ComPtr;

namespace KujataEngine {

// 4つのシェーダーのLineConstantsと並びを合わせ、HLSLの16バイトの区切りに合うように詰め物を入れる。
struct LineConstants {
	Matrix4x4 viewProjection;
	Matrix4x4 view;
	Matrix4x4 projection;
	float viewportWidth;
	float viewportHeight;
	float padding[2];
};
static_assert(sizeof(LineConstants) == 208, "LineConstantsの大きさが変わったら線のシェーダーも直す。");
static_assert(sizeof(LineVertex) == 28, "LineVertexの大きさが変わったら入力の設定とLine.VS.hlslも直す。");
static_assert(sizeof(ThickLineSegment) == 48, "ThickLineSegmentの大きさが変わったらThickLine.VS.hlslも直す。");

// 太い線1本を、三角形2つ分の6頂点で描く。
static const UINT kVerticesPerThickLine = 6;

void LinePass::Initialize(IGpu* gpu, ShaderCompiler* shaderCompiler, const std::filesystem::path& resourcesDirectory, const LineBatches* batches) {
	batches_ = batches;

	const std::filesystem::path shaderDirectory = resourcesDirectory / L"Shaders";
	ComPtr<IDxcBlob> thinVertexShader = shaderCompiler->Compile((shaderDirectory / L"Line.VS.hlsl").wstring(), L"vs_6_0");
	ComPtr<IDxcBlob> thinPixelShader = shaderCompiler->Compile((shaderDirectory / L"Line.PS.hlsl").wstring(), L"ps_6_0");
	ComPtr<IDxcBlob> thickVertexShader = shaderCompiler->Compile((shaderDirectory / L"ThickLine.VS.hlsl").wstring(), L"vs_6_0");
	ComPtr<IDxcBlob> thickPixelShader = shaderCompiler->Compile((shaderDirectory / L"ThickLine.PS.hlsl").wstring(), L"ps_6_0");

	// シェーダーの書き間違いは普通に起きるので、止めずに描かないだけにする。
	if (thinVertexShader == nullptr || thinPixelShader == nullptr || thickVertexShader == nullptr || thickPixelShader == nullptr) {
		printf("線のシェーダーを準備できなかったので、線を描きません。\n");
		return;
	}

	ID3D12Device* device = gpu->GetDevice();
	CreateRootSignature(device);
	thinDepthTestedState_ = CreatePipelineState(gpu, thinVertexShader.Get(), thinPixelShader.Get(), true, true);
	thinAlwaysVisibleState_ = CreatePipelineState(gpu, thinVertexShader.Get(), thinPixelShader.Get(), true, false);
	thickDepthTestedState_ = CreatePipelineState(gpu, thickVertexShader.Get(), thickPixelShader.Get(), false, true);
	thickAlwaysVisibleState_ = CreatePipelineState(gpu, thickVertexShader.Get(), thickPixelShader.Get(), false, false);

	// 毎フレーム書き込むので、CPUから書けるバッファを上限の分だけ作り、開いたままにする。
	thinVertexBuffer_ = CreateUploadBuffer(device, sizeof(LineVertex) * 2 * kMaxThinLineCount);
	HRESULT result = thinVertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&thinVertices_));
	KUJATA_ASSERT(SUCCEEDED(result));

	thickLineBuffer_ = CreateUploadBuffer(device, sizeof(ThickLineSegment) * kMaxThickLineCount);
	result = thickLineBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&thickLines_));
	KUJATA_ASSERT(SUCCEEDED(result));

	constantBuffer_.Initialize(device, sizeof(LineConstants), 1);
	isReady_ = true;
}

void LinePass::Execute(const RenderContext& context) {
	if (!isReady_) {
		return;
	}

	const UINT thinDepthTestedCount = static_cast<UINT>(batches_->thinDepthTested.size());
	const UINT thinAlwaysVisibleCount = static_cast<UINT>(batches_->thinAlwaysVisible.size());
	const UINT thickDepthTestedCount = static_cast<UINT>(batches_->thickDepthTested.size());
	const UINT thickAlwaysVisibleCount = static_cast<UINT>(batches_->thickAlwaysVisible.size());
	if (thinDepthTestedCount + thinAlwaysVisibleCount + thickDepthTestedCount + thickAlwaysVisibleCount == 0) {
		return;
	}

	// Gpuが毎フレーム完了を待つので、前のフレームの線を上書きしてもGPUはもう読んでいない。
	// 深度テストありの線を前に、なしの線を後ろに続けて置き、描くときは始める位置をずらす。
	std::memcpy(thinVertices_, batches_->thinDepthTested.data(), sizeof(LineVertex) * thinDepthTestedCount);
	std::memcpy(thinVertices_ + thinDepthTestedCount, batches_->thinAlwaysVisible.data(), sizeof(LineVertex) * thinAlwaysVisibleCount);
	std::memcpy(thickLines_, batches_->thickDepthTested.data(), sizeof(ThickLineSegment) * thickDepthTestedCount);
	std::memcpy(thickLines_ + thickDepthTestedCount, batches_->thickAlwaysVisible.data(), sizeof(ThickLineSegment) * thickAlwaysVisibleCount);

	LineConstants constants = {};
	constants.viewProjection = context.viewProjection;
	constants.view = context.view;
	constants.projection = context.projection;
	constants.viewportWidth = context.targetWidth;
	constants.viewportHeight = context.targetHeight;
	constantBuffer_.Write(0, &constants);

	ID3D12GraphicsCommandList* list = context.commandList;

	// 工程ごとに独立させるため、前の工程と同じ範囲をここでも設定する。
	const D3D12_VIEWPORT viewport = { 0.0f, 0.0f, context.targetWidth, context.targetHeight, 0.0f, 1.0f };
	const D3D12_RECT scissor = { 0, 0, static_cast<LONG>(context.targetWidth), static_cast<LONG>(context.targetHeight) };
	list->RSSetViewports(1, &viewport);
	list->RSSetScissorRects(1, &scissor);

	list->SetGraphicsRootSignature(rootSignature_.Get());
	constantBuffer_.Bind(list, kConstantsRootParameterIndex, 0);

	if (thinDepthTestedCount + thinAlwaysVisibleCount > 0) {
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
		vertexBufferView.BufferLocation = thinVertexBuffer_->GetGPUVirtualAddress();
		vertexBufferView.SizeInBytes = sizeof(LineVertex) * (thinDepthTestedCount + thinAlwaysVisibleCount);
		vertexBufferView.StrideInBytes = sizeof(LineVertex);
		list->IASetVertexBuffers(0, 1, &vertexBufferView);
		list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

		if (thinDepthTestedCount > 0) {
			list->SetPipelineState(thinDepthTestedState_.Get());
			list->DrawInstanced(thinDepthTestedCount, 1, 0, 0);
		}
		if (thinAlwaysVisibleCount > 0) {
			list->SetPipelineState(thinAlwaysVisibleState_.Get());
			list->DrawInstanced(thinAlwaysVisibleCount, 1, thinDepthTestedCount, 0);
		}
	}

	if (thickDepthTestedCount + thickAlwaysVisibleCount > 0) {
		// 頂点の入力は使わず、頂点シェーダーが頂点の番号から何本目の線のどの角かを求める。
		list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		const D3D12_GPU_VIRTUAL_ADDRESS thickLinesAddress = thickLineBuffer_->GetGPUVirtualAddress();

		if (thickDepthTestedCount > 0) {
			list->SetPipelineState(thickDepthTestedState_.Get());
			list->SetGraphicsRootShaderResourceView(kThickLinesRootParameterIndex, thickLinesAddress);
			list->DrawInstanced(thickDepthTestedCount * kVerticesPerThickLine, 1, 0, 0);
		}
		if (thickAlwaysVisibleCount > 0) {
			list->SetPipelineState(thickAlwaysVisibleState_.Get());
			list->SetGraphicsRootShaderResourceView(kThickLinesRootParameterIndex, thickLinesAddress + sizeof(ThickLineSegment) * thickDepthTestedCount);
			list->DrawInstanced(thickAlwaysVisibleCount * kVerticesPerThickLine, 1, 0, 0);
		}
	}
}

void LinePass::CreateRootSignature(ID3D12Device* device) {
	D3D12_ROOT_PARAMETER rootParameters[2] = {};

	// b0で、頂点シェーダーがカメラの行列と画面の大きさを受け取る。
	rootParameters[kConstantsRootParameterIndex].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[kConstantsRootParameterIndex].Descriptor.ShaderRegister = 0;
	rootParameters[kConstantsRootParameterIndex].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	// t0で、太い線の頂点シェーダーが線の一覧を受け取る。名札の棚を使わず、バッファの場所を直接渡す。
	rootParameters[kThickLinesRootParameterIndex].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootParameters[kThickLinesRootParameterIndex].Descriptor.ShaderRegister = 0;
	rootParameters[kThickLinesRootParameterIndex].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	// 細い線は頂点の入力を使うので、入力の設定を許す旗を立てる。
	D3D12_ROOT_SIGNATURE_DESC description = {};
	description.NumParameters = _countof(rootParameters);
	description.pParameters = rootParameters;
	description.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignature_ = CreateRootSignatureFromDescription(device, description);
}

ComPtr<ID3D12PipelineState> LinePass::CreatePipelineState(IGpu* gpu, IDxcBlob* vertexShader, IDxcBlob* pixelShader, bool isThin, bool depthTest) {
	// LineVertexとLine.VS.hlslのVertexInputと並びを合わせる。
	D3D12_INPUT_ELEMENT_DESC inputElements[2] = {};
	inputElements[0].SemanticName = "POSITION";
	inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElements[0].AlignedByteOffset = offsetof(LineVertex, position);
	inputElements[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	inputElements[1].SemanticName = "COLOR";
	inputElements[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElements[1].AlignedByteOffset = offsetof(LineVertex, color);
	inputElements[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC description = {};
	description.pRootSignature = rootSignature_.Get();
	description.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
	description.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };

	// 太い線は頂点の入力を使わない。
	if (isThin) {
		description.InputLayout = { inputElements, _countof(inputElements) };
		description.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	} else {
		description.InputLayout = { nullptr, 0 };
		description.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	}

	// 色のアルファで下と混ぜ、薄い線は透けて見えるようにする。画面のアルファは変えない。
	D3D12_RENDER_TARGET_BLEND_DESC& blend = description.BlendState.RenderTarget[0];
	blend.BlendEnable = TRUE;
	blend.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blend.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blend.BlendOp = D3D12_BLEND_OP_ADD;
	blend.SrcBlendAlpha = D3D12_BLEND_ZERO;
	blend.DestBlendAlpha = D3D12_BLEND_ONE;
	blend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	description.SampleMask = UINT32_MAX;

	// 板の表と裏を気にしなくて済むように、どちら向きの三角形も描く。
	description.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	description.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	description.RasterizerState.DepthClipEnable = TRUE;
	description.NumRenderTargets = 1;
	description.RTVFormats[0] = gpu->GetRenderTargetFormat();

	// 深度は読むだけで書かないので、線は物に隠れるが、線どうしや後の描画は隠さない。
	description.DepthStencilState.DepthEnable = depthTest ? TRUE : FALSE;
	description.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	description.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	description.DSVFormat = gpu->GetDepthFormat();
	description.SampleDesc.Count = 1;

	ComPtr<ID3D12PipelineState> pipelineState;
	const HRESULT result = gpu->GetDevice()->CreateGraphicsPipelineState(&description, IID_PPV_ARGS(&pipelineState));
	KUJATA_ASSERT(SUCCEEDED(result));
	return pipelineState;
}

} // namespace KujataEngine
