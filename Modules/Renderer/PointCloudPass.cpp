#include "PointCloudPass.h"

#include "Core/Assert.h"
#include "Core/Math/MathUtil.h"
#include "UploadBuffer.h"

#include <cstdio>
#include <cstring>
#include <vector>

using Microsoft::WRL::ComPtr;

namespace KujataEngine {

// PointCloud.VS.hlslとPointCloudUpdate.CS.hlslのPointと並びを合わせる。
struct Point {
	Vector3 position;
	Vector3 velocity;
};
static_assert(sizeof(Point) == 24, "Pointの大きさが変わったらPointCloud.VS.hlslとPointCloudUpdate.CS.hlslも直す。");

// PointCloud.VS.hlslとPointCloud.PS.hlslのPointCloudConstantsと並びを合わせ、HLSLの16バイトの区切りに合うように詰め物を入れる。
struct PointCloudConstants {
	Matrix4x4 view;
	Matrix4x4 projection;
	float pointSize;
	float intensity;
	float padding[2];
};
static_assert(sizeof(PointCloudConstants) == 144, "PointCloudConstantsの大きさが変わったらPointCloud.VS.hlslとPointCloud.PS.hlslも直す。");

// PointCloudUpdate.CS.hlslのUpdateConstantsと並びを合わせ、float3の後ろに4バイトの値を置いてHLSLの16バイトの区切りに合わせる。
struct PointUpdateConstants {
	Vector3 boxCenter;
	float deltaTime;
	Vector3 boxHalfSize;
	uint32_t pointCount;
	uint32_t frameIndex;
};
static_assert(sizeof(PointUpdateConstants) == 36, "PointUpdateConstantsの大きさが変わったらPointCloudUpdate.CS.hlslも直す。");

// ルート定数は4バイトずつ数える。
static const UINT kUpdateConstantsCount = sizeof(PointUpdateConstants) / sizeof(uint32_t);

// 横と奥に並べる点の数で、掛けた数が全体の数になる。
static const UINT kGridWidth = 1000;
static const UINT kGridDepth = 1000;
static const UINT kPointCount = kGridWidth * kGridDepth;

// 点と点の間隔と、板の中心から端までの長さ。
static const float kPointSpacing = 0.005f;
static const float kPointSize = 0.01f;

// 点1つが足す明るさで、点が重なる数だけ足されるので小さくする。
static const float kPointIntensity = 0.1f;

// 格子の真ん中で、デモの自機のまわりの床から少し浮かせた位置。
static const Vector3 kGridCenter = { 0.0f, 1.2f, -1.0f };

// 点が動ける箱の、格子の面から上と下への広さ。
static const float kBoxHalfHeight = 0.5f;

// PointCloudUpdate.CS.hlslのnumthreadsと合わせる。
static const UINT kThreadGroupSize = 256;

// 割り切れない分も動かすために切り上げる。
static const UINT kThreadGroupCount = (kPointCount + kThreadGroupSize - 1) / kThreadGroupSize;
static_assert(kThreadGroupCount <= D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION, "1回のDispatchで動かせる数を超えたので、点を減らすか組を大きくする。");

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

void PointCloudPass::Initialize(IGpu* gpu, ShaderCompiler* shaderCompiler, ShaderResourceHeap* shaderResourceHeap, const std::filesystem::path& dataDirectory) {
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

	CreatePointBuffer(gpu);
	CreateRootSignature(gpu->GetDevice());
	CreatePipelineState(gpu, vertexShader.Get(), pixelShader.Get());
	CreateUpdateRootSignature(gpu->GetDevice());
	CreateUpdatePipelineState(gpu->GetDevice(), computeShader.Get());
	constantBuffer_.Initialize(gpu->GetDevice(), sizeof(PointCloudConstants), 1);
	gpuTimer_.Initialize(gpu, kMarkCount);
	isReady_ = true;
}

void PointCloudPass::Execute(const RenderContext& context) {
	if (!isReady_) {
		return;
	}

	ReportGpuTime();

	ID3D12GraphicsCommandList* list = context.commandList;
	gpuTimer_.Mark(list, kUpdateBeginMark);
	UpdatePoints(context);
	gpuTimer_.Mark(list, kUpdateEndMark);

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
	constants.pointSize = kPointSize;
	constants.intensity = kPointIntensity;
	constantBuffer_.Write(0, &constants);

	list->SetPipelineState(pipelineState_.Get());
	list->SetGraphicsRootSignature(rootSignature_.Get());
	shaderResourceHeap_->Bind(list);
	constantBuffer_.Bind(list, kConstantsRootParameterIndex, 0);
	list->SetGraphicsRootDescriptorTable(kPointsRootParameterIndex, pointsGpuHandle_);
	list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// 頂点4つの板を、点の数だけ繰り返して描く。
	list->DrawInstanced(4, kPointCount, 0, 0);

	gpuTimer_.Mark(list, kDrawEndMark);
	gpuTimer_.Resolve(list);
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
	printf("点%u個のGPU時間: 更新 %.3f ms、描画 %.3f ms、合計 %.3f ms\n", kPointCount, updateMilliseconds, drawMilliseconds, updateMilliseconds + drawMilliseconds);

	updateMillisecondsSum_ = 0.0;
	drawMillisecondsSum_ = 0.0;
	measuredFrameCount_ = 0;
}

void PointCloudPass::UpdatePoints(const RenderContext& context) {
	ID3D12GraphicsCommandList* list = context.commandList;

	PointUpdateConstants constants = {};
	constants.boxCenter = kGridCenter;
	constants.deltaTime = context.deltaTime;
	constants.boxHalfSize = { kGridWidth * kPointSpacing * 0.5f, kBoxHalfHeight, kGridDepth * kPointSpacing * 0.5f };
	constants.pointCount = kPointCount;
	constants.frameIndex = frameIndex_;
	frameIndex_++;

	list->SetPipelineState(updatePipelineState_.Get());
	list->SetComputeRootSignature(updateRootSignature_.Get());
	shaderResourceHeap_->Bind(list);
	list->SetComputeRoot32BitConstants(kUpdateConstantsRootParameterIndex, kUpdateConstantsCount, &constants, 0);
	list->SetComputeRootDescriptorTable(kUpdatePointsRootParameterIndex, pointsUavGpuHandle_);

	// 前のフレームの命令が終わるとバッファはCOMMONに戻るので、毎フレームCOMMONから書き込める状態にする。
	TransitionPointBuffer(list, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	list->Dispatch(kThreadGroupCount, 1, 1);

	// 頂点シェーダーが読む前に、書き込みが終わるのを待たせる。
	TransitionPointBuffer(list, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}

void PointCloudPass::TransitionPointBuffer(ID3D12GraphicsCommandList* list, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after) {
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = pointBuffer_.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = before;
	barrier.Transition.StateAfter = after;
	list->ResourceBarrier(1, &barrier);
}

void PointCloudPass::CreatePointBuffer(IGpu* gpu) {
	ID3D12Device* device = gpu->GetDevice();

	// 速度は0から始め、コンピュートシェーダーが揺らしていく。
	std::vector<Point> points;
	points.reserve(kPointCount);
	for (UINT z = 0; z < kGridDepth; z++) {
		for (UINT x = 0; x < kGridWidth; x++) {
			Point point = {};
			point.position.x = (static_cast<float>(x) - (kGridWidth - 1) * 0.5f) * kPointSpacing + kGridCenter.x;
			point.position.y = kGridCenter.y;
			point.position.z = (static_cast<float>(z) - (kGridDepth - 1) * 0.5f) * kPointSpacing + kGridCenter.z;
			points.push_back(point);
		}
	}

	const UINT bufferSize = sizeof(Point) * kPointCount;

	// GPUだけが読み書きするメモリに作り、コンピュートシェーダーからも書き込めるようにUAVを許す。
	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_RESOURCE_DESC description = {};
	description.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	description.Width = bufferSize;
	description.Height = 1;
	description.DepthOrArraySize = 1;
	description.MipLevels = 1;
	description.SampleDesc.Count = 1;
	description.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	description.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	// バッファはCOMMONで作れば、コピーや読み込みのときにGPUが必要な状態へ自動で切り替える。
	HRESULT result = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &description, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&pointBuffer_));
	KUJATA_ASSERT(SUCCEEDED(result));

	// CPUから書ける一時バッファに入れてから、GPUの中でコピーする。
	ComPtr<ID3D12Resource> uploadBuffer = CreateUploadBuffer(device, bufferSize);
	void* mappedMemory = nullptr;
	result = uploadBuffer->Map(0, nullptr, &mappedMemory);
	KUJATA_ASSERT(SUCCEEDED(result));
	memcpy(mappedMemory, points.data(), bufferSize);
	uploadBuffer->Unmap(0, nullptr);

	// 完了まで待つので、関数を抜けて一時バッファが消えても問題ない。
	gpu->ExecuteImmediately([&](ID3D12GraphicsCommandList* list) {
		list->CopyBufferRegion(pointBuffer_.Get(), 0, uploadBuffer.Get(), 0, bufferSize);
	});

	// シェーダーのStructuredBuffer<Point>として読むための名札を、棚に置く。
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescription = {};
	srvDescription.Format = DXGI_FORMAT_UNKNOWN;
	srvDescription.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescription.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDescription.Buffer.FirstElement = 0;
	srvDescription.Buffer.NumElements = kPointCount;
	srvDescription.Buffer.StructureByteStride = sizeof(Point);

	const UINT srvHeapIndex = shaderResourceHeap_->Allocate();
	device->CreateShaderResourceView(pointBuffer_.Get(), &srvDescription, shaderResourceHeap_->GetCpuHandle(srvHeapIndex));
	pointsGpuHandle_ = shaderResourceHeap_->GetGpuHandle(srvHeapIndex);

	// コンピュートシェーダーのRWStructuredBuffer<Point>として書くための名札も、棚に置く。
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDescription = {};
	uavDescription.Format = DXGI_FORMAT_UNKNOWN;
	uavDescription.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDescription.Buffer.FirstElement = 0;
	uavDescription.Buffer.NumElements = kPointCount;
	uavDescription.Buffer.StructureByteStride = sizeof(Point);

	const UINT uavHeapIndex = shaderResourceHeap_->Allocate();
	device->CreateUnorderedAccessView(pointBuffer_.Get(), nullptr, &uavDescription, shaderResourceHeap_->GetCpuHandle(uavHeapIndex));
	pointsUavGpuHandle_ = shaderResourceHeap_->GetGpuHandle(uavHeapIndex);

	printf("点を%u個、GPUへ送りました。\n", kPointCount);
}

void PointCloudPass::CreateRootSignature(ID3D12Device* device) {
	D3D12_DESCRIPTOR_RANGE pointsRange = {};
	pointsRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pointsRange.NumDescriptors = 1;
	pointsRange.BaseShaderRegister = 0;
	pointsRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameters[2] = {};

	// b0で、頂点シェーダーがカメラと板の大きさを、ピクセルシェーダーが明るさを受け取る。
	rootParameters[kConstantsRootParameterIndex].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[kConstantsRootParameterIndex].Descriptor.ShaderRegister = 0;
	rootParameters[kConstantsRootParameterIndex].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// 頂点シェーダーのt0で点のバッファを受け取る。
	rootParameters[kPointsRootParameterIndex].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[kPointsRootParameterIndex].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[kPointsRootParameterIndex].DescriptorTable.pDescriptorRanges = &pointsRange;
	rootParameters[kPointsRootParameterIndex].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

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
	D3D12_DESCRIPTOR_RANGE pointsRange = {};
	pointsRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	pointsRange.NumDescriptors = 1;
	pointsRange.BaseShaderRegister = 0;
	pointsRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// コンピュートシェーダーには見える範囲の区別がないので、どの入口もALLにする。
	D3D12_ROOT_PARAMETER rootParameters[2] = {};

	// b0で毎フレームの秒数や箱の大きさを、定数バッファを作らずに直接受け取る。
	rootParameters[kUpdateConstantsRootParameterIndex].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParameters[kUpdateConstantsRootParameterIndex].Constants.ShaderRegister = 0;
	rootParameters[kUpdateConstantsRootParameterIndex].Constants.Num32BitValues = kUpdateConstantsCount;
	rootParameters[kUpdateConstantsRootParameterIndex].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// u0で点のバッファに書き込む。
	rootParameters[kUpdatePointsRootParameterIndex].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[kUpdatePointsRootParameterIndex].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[kUpdatePointsRootParameterIndex].DescriptorTable.pDescriptorRanges = &pointsRange;
	rootParameters[kUpdatePointsRootParameterIndex].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

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
