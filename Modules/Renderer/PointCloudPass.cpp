#include "PointCloudPass.h"

#include "Core/Assert.h"
#include "Core/Math/MathUtil.h"
#include "UploadBuffer.h"

#include <cstdio>
#include <cstring>
#include <vector>

using Microsoft::WRL::ComPtr;

namespace KujataEngine {

// PointCloud.VS.hlslのPointと並びを合わせる。
struct Point {
	Vector3 position;
};
static_assert(sizeof(Point) == 12, "Pointの大きさが変わったらPointCloud.VS.hlslも直す。");

// PointCloud.VS.hlslのCameraDataと並びを合わせ、HLSLの16バイトの区切りに合うように詰め物を入れる。
struct PointCloudConstants {
	Matrix4x4 view;
	Matrix4x4 projection;
	float pointSize;
	float padding[3];
};
static_assert(sizeof(PointCloudConstants) == 144, "PointCloudConstantsの大きさが変わったらPointCloud.VS.hlslも直す。");

// 横と奥に並べる点の数で、掛けた数が全体の数になる。
static const UINT kGridWidth = 4000;
static const UINT kGridDepth = 2500;
static const UINT kPointCount = kGridWidth * kGridDepth;

// 点と点の間隔と、板の中心から端までの長さ。
static const float kPointSpacing = 0.005f;
static const float kPointSize = 0.01f;

void PointCloudPass::Initialize(IGpu* gpu, ShaderCompiler* shaderCompiler, ShaderResourceHeap* shaderResourceHeap, const std::filesystem::path& dataDirectory) {
	shaderResourceHeap_ = shaderResourceHeap;

	const std::filesystem::path shaderDirectory = dataDirectory / L"Shaders";
	ComPtr<IDxcBlob> vertexShader = shaderCompiler->Compile((shaderDirectory / L"PointCloud.VS.hlsl").wstring(), L"vs_6_0");
	ComPtr<IDxcBlob> pixelShader = shaderCompiler->Compile((shaderDirectory / L"PointCloud.PS.hlsl").wstring(), L"ps_6_0");

	// シェーダーの書き間違いは普通に起きるので、止めずに描かないだけにする。
	if (vertexShader == nullptr || pixelShader == nullptr) {
		printf("点のシェーダーを準備できなかったので、点を描きません。\n");
		return;
	}

	CreatePointBuffer(gpu);
	CreateRootSignature(gpu->GetDevice());
	CreatePipelineState(gpu, vertexShader.Get(), pixelShader.Get());
	constantBuffer_.Initialize(gpu->GetDevice(), sizeof(PointCloudConstants), 1);
	isReady_ = true;
}

void PointCloudPass::Execute(const RenderContext& context) {
	if (!isReady_) {
		return;
	}

	ID3D12GraphicsCommandList* list = context.commandList;

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
	constantBuffer_.Write(0, &constants);

	list->SetPipelineState(pipelineState_.Get());
	list->SetGraphicsRootSignature(rootSignature_.Get());
	shaderResourceHeap_->Bind(list);
	constantBuffer_.Bind(list, kConstantsRootParameterIndex, 0);
	list->SetGraphicsRootDescriptorTable(kPointsRootParameterIndex, pointsGpuHandle_);
	list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// 頂点4つの板を、点の数だけ繰り返して描く。
	list->DrawInstanced(4, kPointCount, 0, 0);
}

void PointCloudPass::CreatePointBuffer(IGpu* gpu) {
	ID3D12Device* device = gpu->GetDevice();

	// デモの自機のまわりに、床から少し浮かせて格子状に並べる。
	std::vector<Point> points;
	points.reserve(kPointCount);
	for (UINT z = 0; z < kGridDepth; z++) {
		for (UINT x = 0; x < kGridWidth; x++) {
			Point point = {};
			point.position.x = (static_cast<float>(x) - (kGridWidth - 1) * 0.5f) * kPointSpacing;
			point.position.y = 1.2f;
			point.position.z = (static_cast<float>(z) - (kGridDepth - 1) * 0.5f) * kPointSpacing - 1.0f;
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
	D3D12_SHADER_RESOURCE_VIEW_DESC viewDescription = {};
	viewDescription.Format = DXGI_FORMAT_UNKNOWN;
	viewDescription.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	viewDescription.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	viewDescription.Buffer.FirstElement = 0;
	viewDescription.Buffer.NumElements = kPointCount;
	viewDescription.Buffer.StructureByteStride = sizeof(Point);

	const UINT heapIndex = shaderResourceHeap_->Allocate();
	device->CreateShaderResourceView(pointBuffer_.Get(), &viewDescription, shaderResourceHeap_->GetCpuHandle(heapIndex));
	pointsGpuHandle_ = shaderResourceHeap_->GetGpuHandle(heapIndex);

	printf("点を%u個、GPUへ送りました。\n", kPointCount);
}

void PointCloudPass::CreateRootSignature(ID3D12Device* device) {
	D3D12_DESCRIPTOR_RANGE pointsRange = {};
	pointsRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pointsRange.NumDescriptors = 1;
	pointsRange.BaseShaderRegister = 0;
	pointsRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameters[2] = {};

	// 頂点シェーダーのb0でカメラと板の大きさを受け取る。
	rootParameters[kConstantsRootParameterIndex].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[kConstantsRootParameterIndex].Descriptor.ShaderRegister = 0;
	rootParameters[kConstantsRootParameterIndex].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

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

	ComPtr<ID3DBlob> serialized;
	ComPtr<ID3DBlob> errors;
	HRESULT result = D3D12SerializeRootSignature(&description, D3D_ROOT_SIGNATURE_VERSION_1, &serialized, &errors);
	KUJATA_ASSERT(SUCCEEDED(result));

	result = device->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	KUJATA_ASSERT(SUCCEEDED(result));
}

void PointCloudPass::CreatePipelineState(IGpu* gpu, IDxcBlob* vertexShader, IDxcBlob* pixelShader) {
	D3D12_GRAPHICS_PIPELINE_STATE_DESC description = {};
	description.pRootSignature = rootSignature_.Get();
	description.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
	description.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };

	// 頂点の入力はなく、SV_VertexIDとSV_InstanceIDだけで描く。
	description.InputLayout = { nullptr, 0 };

	description.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	description.SampleMask = UINT32_MAX;
	description.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;

	// 板の表と裏を気にしなくて済むように、どちら向きの三角形も描く。
	description.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	description.RasterizerState.DepthClipEnable = TRUE;
	description.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	description.NumRenderTargets = 1;
	description.RTVFormats[0] = gpu->GetRenderTargetFormat();

	// 不透明な物と同じく、深度を読み書きする。
	description.DepthStencilState.DepthEnable = TRUE;
	description.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	description.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	description.DSVFormat = gpu->GetDepthFormat();
	description.SampleDesc.Count = 1;

	const HRESULT result = gpu->GetDevice()->CreateGraphicsPipelineState(&description, IID_PPV_ARGS(&pipelineState_));
	KUJATA_ASSERT(SUCCEEDED(result));
}

} // namespace KujataEngine
