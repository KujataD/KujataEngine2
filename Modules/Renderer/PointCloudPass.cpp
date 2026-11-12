#include "PointCloudPass.h"

#include "Core/Assert.h"
#include "Core/Math/MathUtil.h"
#include "GpuBuffer.h"

#include <algorithm>
#include <cstdio>

using Microsoft::WRL::ComPtr;

namespace KujataEngine {

// PointCloud.VS.hlslとPointCloudUpdate.CS.hlslのPointStateと並びを合わせる。
struct PointState {
	Vector3 position;
	Vector3 velocity;
};
static_assert(sizeof(PointState) == 24, "PointStateの大きさが変わったらPointCloud.VS.hlslとPointCloudUpdate.CS.hlslも直す。");

// PointCloud.VS.hlslとPointCloud.PS.hlslのPointCloudConstantsと並びを合わせ、HLSLの16バイトの区切りに合うように詰め物を入れる。
struct PointCloudConstants {
	Matrix4x4 view;
	Matrix4x4 projection;
	float pointSize;
	float intensity;
	uint32_t targetCount;
	float padding;
};
static_assert(sizeof(PointCloudConstants) == 144, "PointCloudConstantsの大きさが変わったらPointCloud.VS.hlslとPointCloud.PS.hlslも直す。");

// PointCloudUpdate.CS.hlslのUpdateConstantsと並びを合わせる。
struct PointUpdateConstants {
	Matrix4x4 world;
	float deltaTime;
	uint32_t pointCount;
	uint32_t targetCount;
	uint32_t frameIndex;
	uint32_t shouldInitialize;
};
static_assert(sizeof(PointUpdateConstants) == 84, "PointUpdateConstantsの大きさが変わったらPointCloudUpdate.CS.hlslも直す。");

// ルート定数は4バイトずつ数える。
static const UINT kUpdateConstantsCount = sizeof(PointUpdateConstants) / sizeof(uint32_t);

// 1フレームに動かして描ける点群の数で、定数バッファの場所の数になる。
static const UINT kMaxPointCloudCount = 16;

// PointCloudUpdate.CS.hlslのnumthreadsと合わせる。
static const UINT kThreadGroupSize = 256;

// 1回のDispatchで動かせる組の数に上限があるので、1つの点群の点の数もここまでにする。
static const UINT kMaxPointCountPerCloud = D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION * kThreadGroupSize;

// GPU時間を測る印の番号で、点を動かす前、動かした後、描いた後に付ける。
static const UINT kUpdateBeginMark = 0;
static const UINT kUpdateEndMark = 1;
static const UINT kDrawEndMark = 2;
static const UINT kMarkCount = 3;

// GPU時間の平均をログに出す間隔で、単位はフレーム。
static const uint32_t kGpuTimeReportFrameCount = 120;

/// <summary>
/// 入口の一覧の説明からRootSignatureを作る。
/// </summary>
static ComPtr<ID3D12RootSignature> CreateRootSignatureFromDescription(ID3D12Device* device, const D3D12_ROOT_SIGNATURE_DESC& description) {
	ComPtr<ID3DBlob> serialized;
	ComPtr<ID3DBlob> errors;
	HRESULT result = D3D12SerializeRootSignature(&description, D3D_ROOT_SIGNATURE_VERSION_1, &serialized, &errors);
	KUJATA_ASSERT(SUCCEEDED(result));

	ComPtr<ID3D12RootSignature> rootSignature;
	result = device->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	KUJATA_ASSERT(SUCCEEDED(result));
	return rootSignature;
}

/// <summary>
/// 名札を1つだけ受け取るテーブルの入口を作る。
/// </summary>
static D3D12_ROOT_PARAMETER MakeTableParameter(const D3D12_DESCRIPTOR_RANGE* range, D3D12_SHADER_VISIBILITY visibility) {
	D3D12_ROOT_PARAMETER parameter = {};
	parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	parameter.DescriptorTable.NumDescriptorRanges = 1;
	parameter.DescriptorTable.pDescriptorRanges = range;
	parameter.ShaderVisibility = visibility;
	return parameter;
}

/// <summary>
/// 指定の種類とレジスタの番号の名札1つ分の範囲を作る。
/// </summary>
static D3D12_DESCRIPTOR_RANGE MakeSingleRange(D3D12_DESCRIPTOR_RANGE_TYPE type, UINT shaderRegister) {
	D3D12_DESCRIPTOR_RANGE range = {};
	range.RangeType = type;
	range.NumDescriptors = 1;
	range.BaseShaderRegister = shaderRegister;
	range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	return range;
}

void PointCloudPass::Initialize(IGpu* gpu, ShaderCompiler* shaderCompiler, ShaderResourceHeap* shaderResourceHeap, const std::filesystem::path& dataDirectory) {
	gpu_ = gpu;
	shaderResourceHeap_ = shaderResourceHeap;

	const std::filesystem::path shaderDirectory = dataDirectory / L"Shaders";
	ComPtr<IDxcBlob> vertexShader = shaderCompiler->Compile((shaderDirectory / L"PointCloud.VS.hlsl").wstring(), L"vs_6_0");
	ComPtr<IDxcBlob> pixelShader = shaderCompiler->Compile((shaderDirectory / L"PointCloud.PS.hlsl").wstring(), L"ps_6_0");
	ComPtr<IDxcBlob> computeShader = shaderCompiler->Compile((shaderDirectory / L"PointCloudUpdate.CS.hlsl").wstring(), L"cs_6_0");

	// シェーダーの書き間違いは普通に起きるので、止めずに描かないだけにする。
	if (vertexShader == nullptr || pixelShader == nullptr || computeShader == nullptr) {
		printf("点のシェーダーを準備できなかったので、点を描きません。\n");
		return;
	}

	CreateRootSignature(gpu->GetDevice());
	CreatePipelineState(gpu, vertexShader.Get(), pixelShader.Get());
	CreateUpdateRootSignature(gpu->GetDevice());
	CreateUpdatePipelineState(gpu->GetDevice(), computeShader.Get());
	constantBuffer_.Initialize(gpu->GetDevice(), sizeof(PointCloudConstants), kMaxPointCloudCount);
	gpuTimer_.Initialize(gpu, kMarkCount);
	isReady_ = true;
}

void PointCloudPass::Execute(const RenderContext& context) {
	if (!isReady_) {
		return;
	}

	ReportGpuTime();

	// 点群の数はゲームしだいで増えるので、止めずに上限までだけを描く。
	const std::vector<PointCloudRequest>& requests = context.renderQueue->GetPointCloudRequests();
	if (requests.size() > kMaxPointCloudCount && !hasWarnedTooManyPointClouds_) {
		printf("点群が%u個を超えたので、超えた分は描きません。\n", kMaxPointCloudCount);
		hasWarnedTooManyPointClouds_ = true;
	}
	const UINT requestCount = static_cast<UINT>((std::min)(requests.size(), static_cast<size_t>(kMaxPointCloudCount)));

	// 描く命令の中で探し直さないように、依頼と同じ並びで点のバッファを集める。
	std::vector<PointCloudInstance*> requestInstances;
	lastPointCount_ = 0;
	for (UINT i = 0; i < requestCount; i++) {
		const UINT targetCount = context.gpuAssets->GetPointCloud(requests[i].targets).targetCount;
		PointCloudInstance& instance = FindOrCreateInstance(requests[i], targetCount);
		instance.isUsedThisFrame = true;
		requestInstances.push_back(&instance);
		lastPointCount_ += instance.pointCount;
	}

	// 動かす命令をまとめて先に書き、設定の切り替えを減らす。
	ID3D12GraphicsCommandList* list = context.commandList;
	gpuTimer_.Mark(list, kUpdateBeginMark);
	for (UINT i = 0; i < requestCount; i++) {
		UpdatePoints(context, requests[i], *requestInstances[i]);
	}
	gpuTimer_.Mark(list, kUpdateEndMark);

	for (UINT i = 0; i < requestCount; i++) {
		DrawPoints(context, requests[i], *requestInstances[i], i);
	}
	gpuTimer_.Mark(list, kDrawEndMark);
	gpuTimer_.Resolve(list);

	RemoveUnusedInstances();
	frameIndex_++;
}

PointCloudInstance& PointCloudPass::FindOrCreateInstance(const PointCloudRequest& request, UINT targetCount) {
	if (instances_.count(request.instanceId) > 0) {
		return instances_[request.instanceId];
	}

	if (targetCount > kMaxPointCountPerCloud && !hasWarnedTooManyPoints_) {
		printf("1つの点群の点が%u個を超えたので、超えた分は描きません。\n", kMaxPointCountPerCloud);
		hasWarnedTooManyPoints_ = true;
	}

	ID3D12Device* device = gpu_->GetDevice();
	PointCloudInstance& instance = instances_[request.instanceId];
	instance.pointCount = (std::min)(targetCount, kMaxPointCountPerCloud);

	// コンピュートシェーダーから書き込めるようにUAVを許す。
	instance.stateBuffer = CreateGpuBuffer(device, sizeof(PointState) * static_cast<UINT64>(instance.pointCount), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	// 頂点シェーダーのStructuredBuffer<PointState>として読むための名札を、棚に置く。
	D3D12_SHADER_RESOURCE_VIEW_DESC readDescription = {};
	readDescription.Format = DXGI_FORMAT_UNKNOWN;
	readDescription.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	readDescription.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	readDescription.Buffer.FirstElement = 0;
	readDescription.Buffer.NumElements = instance.pointCount;
	readDescription.Buffer.StructureByteStride = sizeof(PointState);
	instance.readHeapIndex = AllocateHeapIndex();
	device->CreateShaderResourceView(instance.stateBuffer.Get(), &readDescription, shaderResourceHeap_->GetCpuHandle(instance.readHeapIndex));

	// コンピュートシェーダーのRWStructuredBuffer<PointState>として書くための名札も、棚に置く。
	D3D12_UNORDERED_ACCESS_VIEW_DESC writeDescription = {};
	writeDescription.Format = DXGI_FORMAT_UNKNOWN;
	writeDescription.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	writeDescription.Buffer.FirstElement = 0;
	writeDescription.Buffer.NumElements = instance.pointCount;
	writeDescription.Buffer.StructureByteStride = sizeof(PointState);
	instance.writeHeapIndex = AllocateHeapIndex();
	device->CreateUnorderedAccessView(instance.stateBuffer.Get(), nullptr, &writeDescription, shaderResourceHeap_->GetCpuHandle(instance.writeHeapIndex));

	return instance;
}

void PointCloudPass::UpdatePoints(const RenderContext& context, const PointCloudRequest& request, PointCloudInstance& instance) {
	ID3D12GraphicsCommandList* list = context.commandList;
	const GpuPointCloud& targets = context.gpuAssets->GetPointCloud(request.targets);

	PointUpdateConstants constants = {};
	constants.world = request.world;
	constants.deltaTime = context.deltaTime;
	constants.pointCount = instance.pointCount;
	constants.targetCount = targets.targetCount;
	constants.frameIndex = frameIndex_;
	constants.shouldInitialize = instance.shouldInitialize ? 1 : 0;
	instance.shouldInitialize = false;

	list->SetPipelineState(updatePipelineState_.Get());
	list->SetComputeRootSignature(updateRootSignature_.Get());
	shaderResourceHeap_->Bind(list);
	list->SetComputeRoot32BitConstants(kUpdateConstantsRootParameterIndex, kUpdateConstantsCount, &constants, 0);
	list->SetComputeRootDescriptorTable(kUpdatePointsRootParameterIndex, shaderResourceHeap_->GetGpuHandle(instance.writeHeapIndex));
	list->SetComputeRootDescriptorTable(kUpdateTargetsRootParameterIndex, targets.targetsGpuHandle);

	// 前のフレームの命令が終わるとバッファはCOMMONに戻るので、毎フレームCOMMONから書き込める状態にする。
	TransitionStateBuffer(list, instance, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	// 割り切れない分も動かすために切り上げる。
	const UINT threadGroupCount = (instance.pointCount + kThreadGroupSize - 1) / kThreadGroupSize;
	list->Dispatch(threadGroupCount, 1, 1);

	// 頂点シェーダーが読む前に、書き込みが終わるのを待たせる。
	TransitionStateBuffer(list, instance, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}

void PointCloudPass::DrawPoints(const RenderContext& context, const PointCloudRequest& request, const PointCloudInstance& instance, UINT constantsIndex) {
	ID3D12GraphicsCommandList* list = context.commandList;
	const GpuPointCloud& targets = context.gpuAssets->GetPointCloud(request.targets);

	// 工程ごとに独立させるため、前の工程と同じ範囲をここでも設定する。
	const D3D12_VIEWPORT viewport = { 0.0f, 0.0f, context.targetWidth, context.targetHeight, 0.0f, 1.0f };
	const D3D12_RECT scissor = { 0, 0, static_cast<LONG>(context.targetWidth), static_cast<LONG>(context.targetHeight) };
	list->RSSetViewports(1, &viewport);
	list->RSSetScissorRects(1, &scissor);

	const CameraData& camera = context.renderQueue->GetCamera();
	const float aspectRatio = context.targetWidth / context.targetHeight;

	PointCloudConstants constants = {};
	constants.view = camera.view;
	constants.projection = MakePerspectiveFovMatrix(camera.fovYRadians, aspectRatio, camera.nearZ, camera.farZ);
	constants.pointSize = request.pointSize;
	constants.intensity = request.intensity;
	constants.targetCount = targets.targetCount;
	constantBuffer_.Write(constantsIndex, &constants);

	list->SetPipelineState(pipelineState_.Get());
	list->SetGraphicsRootSignature(rootSignature_.Get());
	shaderResourceHeap_->Bind(list);
	constantBuffer_.Bind(list, kConstantsRootParameterIndex, constantsIndex);
	list->SetGraphicsRootDescriptorTable(kPointsRootParameterIndex, shaderResourceHeap_->GetGpuHandle(instance.readHeapIndex));
	list->SetGraphicsRootDescriptorTable(kTargetsRootParameterIndex, targets.targetsGpuHandle);
	list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// 頂点4つの板を、点の数だけ繰り返して描く。
	list->DrawInstanced(4, instance.pointCount, 0, 0);
}

void PointCloudPass::RemoveUnusedInstances() {
	// Gpuが毎フレーム完了を待つので、このフレームで使わなかったバッファはもうGPUも使っていない。
	for (std::map<uint64_t, PointCloudInstance>::iterator it = instances_.begin(); it != instances_.end();) {
		PointCloudInstance& instance = it->second;
		if (instance.isUsedThisFrame) {
			instance.isUsedThisFrame = false;
			++it;
			continue;
		}

		reusableHeapIndices_.push_back(instance.readHeapIndex);
		reusableHeapIndices_.push_back(instance.writeHeapIndex);
		it = instances_.erase(it);
	}
}

UINT PointCloudPass::AllocateHeapIndex() {
	if (reusableHeapIndices_.empty()) {
		return shaderResourceHeap_->Allocate();
	}

	const UINT heapIndex = reusableHeapIndices_.back();
	reusableHeapIndices_.pop_back();
	return heapIndex;
}

void PointCloudPass::ReportGpuTime() {
	// 最初のフレームには、まだ読める印がない。
	if (!gpuTimer_.HasResult()) {
		return;
	}

	updateMillisecondsSum_ += gpuTimer_.ReadMilliseconds(kUpdateBeginMark, kUpdateEndMark);
	drawMillisecondsSum_ += gpuTimer_.ReadMilliseconds(kUpdateEndMark, kDrawEndMark);
	measuredFrameCount_++;
	if (measuredFrameCount_ < kGpuTimeReportFrameCount) {
		return;
	}

	// 垂直同期を待つとfpsでは余裕が見えないので、GPUが実際にかけた時間を出す。
	const double updateMilliseconds = updateMillisecondsSum_ / measuredFrameCount_;
	const double drawMilliseconds = drawMillisecondsSum_ / measuredFrameCount_;
	printf("点%u個のGPU時間: 更新 %.3f ms、描画 %.3f ms、合計 %.3f ms\n", lastPointCount_, updateMilliseconds, drawMilliseconds, updateMilliseconds + drawMilliseconds);

	updateMillisecondsSum_ = 0.0;
	drawMillisecondsSum_ = 0.0;
	measuredFrameCount_ = 0;
}

void PointCloudPass::TransitionStateBuffer(ID3D12GraphicsCommandList* list, const PointCloudInstance& instance, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after) {
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = instance.stateBuffer.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = before;
	barrier.Transition.StateAfter = after;
	list->ResourceBarrier(1, &barrier);
}

void PointCloudPass::CreateRootSignature(ID3D12Device* device) {
	const D3D12_DESCRIPTOR_RANGE pointsRange = MakeSingleRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0);
	const D3D12_DESCRIPTOR_RANGE targetsRange = MakeSingleRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1);

	D3D12_ROOT_PARAMETER rootParameters[3] = {};

	// b0で、頂点シェーダーがカメラと板の大きさを、ピクセルシェーダーが明るさを受け取る。
	rootParameters[kConstantsRootParameterIndex].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[kConstantsRootParameterIndex].Descriptor.ShaderRegister = 0;
	rootParameters[kConstantsRootParameterIndex].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// 頂点シェーダーのt0で点の位置を、t1で集まる先の色を受け取る。
	rootParameters[kPointsRootParameterIndex] = MakeTableParameter(&pointsRange, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[kTargetsRootParameterIndex] = MakeTableParameter(&targetsRange, D3D12_SHADER_VISIBILITY_VERTEX);

	// 頂点バッファを使わないので、入力の設定を許す旗は立てない。
	D3D12_ROOT_SIGNATURE_DESC description = {};
	description.NumParameters = _countof(rootParameters);
	description.pParameters = rootParameters;
	description.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
	rootSignature_ = CreateRootSignatureFromDescription(device, description);
}

void PointCloudPass::CreatePipelineState(IGpu* gpu, IDxcBlob* vertexShader, IDxcBlob* pixelShader) {
	D3D12_GRAPHICS_PIPELINE_STATE_DESC description = {};
	description.pRootSignature = rootSignature_.Get();
	description.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
	description.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };

	// 頂点の入力はなく、SV_VertexIDとSV_InstanceIDだけで描く。
	description.InputLayout = { nullptr, 0 };

	// 重なった点の明るさを足し合わせ、描く順番に左右されないようにする。
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

	// 深度を読むので不透明な物には隠れ、書かないので点どうしは隠し合わない。
	description.DepthStencilState.DepthEnable = TRUE;
	description.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	description.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	description.DSVFormat = gpu->GetDepthFormat();
	description.SampleDesc.Count = 1;

	const HRESULT result = gpu->GetDevice()->CreateGraphicsPipelineState(&description, IID_PPV_ARGS(&pipelineState_));
	KUJATA_ASSERT(SUCCEEDED(result));
}

void PointCloudPass::CreateUpdateRootSignature(ID3D12Device* device) {
	const D3D12_DESCRIPTOR_RANGE pointsRange = MakeSingleRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0);
	const D3D12_DESCRIPTOR_RANGE targetsRange = MakeSingleRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0);

	// コンピュートシェーダーには見える範囲の区別がないので、どの入口もALLにする。
	D3D12_ROOT_PARAMETER rootParameters[3] = {};

	// b0で毎フレームの秒数やワールド行列を、定数バッファを作らずに直接受け取る。
	rootParameters[kUpdateConstantsRootParameterIndex].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParameters[kUpdateConstantsRootParameterIndex].Constants.ShaderRegister = 0;
	rootParameters[kUpdateConstantsRootParameterIndex].Constants.Num32BitValues = kUpdateConstantsCount;
	rootParameters[kUpdateConstantsRootParameterIndex].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// u0で点のバッファに書き込み、t0で集まる先を読む。
	rootParameters[kUpdatePointsRootParameterIndex] = MakeTableParameter(&pointsRange, D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[kUpdateTargetsRootParameterIndex] = MakeTableParameter(&targetsRange, D3D12_SHADER_VISIBILITY_ALL);

	D3D12_ROOT_SIGNATURE_DESC description = {};
	description.NumParameters = _countof(rootParameters);
	description.pParameters = rootParameters;
	description.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
	updateRootSignature_ = CreateRootSignatureFromDescription(device, description);
}

void PointCloudPass::CreateUpdatePipelineState(ID3D12Device* device, IDxcBlob* computeShader) {
	D3D12_COMPUTE_PIPELINE_STATE_DESC description = {};
	description.pRootSignature = updateRootSignature_.Get();
	description.CS = { computeShader->GetBufferPointer(), computeShader->GetBufferSize() };

	const HRESULT result = device->CreateComputePipelineState(&description, IID_PPV_ARGS(&updatePipelineState_));
	KUJATA_ASSERT(SUCCEEDED(result));
}

} // namespace KujataEngine
