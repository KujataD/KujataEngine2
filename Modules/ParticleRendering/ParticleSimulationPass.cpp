#include "ParticleSimulationPass.h"

#include "Core/Assert.h"
#include "Core/Math/Matrix4x4.h"
#include "GraphicsKit/RootSignature.h"
#include "ParticleRendering.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <numbers>

using Microsoft::WRL::ComPtr;

namespace KujataEngine {

// ParticleSimulation.CS.hlslのSimulationConstantsと並びを合わせ、HLSLの16バイトの区切りに合うように詰め物を入れる。
struct SimulationConstants {
	Matrix4x4 world;
	float deltaTime;
	uint32_t pointCount;
	uint32_t targetCount;
	uint32_t shouldInitialize;
	float initialSettle;
	uint32_t gatherPointCount;
	float floatDistance;
	float floatPhase;
	uint32_t hasSettleOverride;
	float settleOverride;
	uint32_t scatterSeed;
	float padding;
	ParticleGatherPoint gatherPoints[kMaxParticleGatherPoints];
};
static_assert(sizeof(SimulationConstants) == 368, "SimulationConstantsの大きさが変わったらParticleSimulation.CS.hlslも直す。");

// 粒の揺れの角度が1秒に進む量で、単位はラジアン。
static const float kFloatRadiansPerSecond = 1.5f;

// 揺れの角度を巻き戻す周期で、シェーダーが掛ける倍率が0.25の倍数なので、ここで戻しても揺れがつながる。
static const float kFloatPhasePeriod = std::numbers::pi_v<float> * 8.0f;

void ParticleSimulationPass::Initialize(IGpu* gpu, ShaderCompiler* shaderCompiler, ShaderResourceHeap* shaderResourceHeap, const std::filesystem::path& resourcesDirectory, const std::vector<ParticleRequest>* requests, ParticleShapeTable* shapes, ParticleInstances* instances) {
	shaderResourceHeap_ = shaderResourceHeap;
	requests_ = requests;
	shapes_ = shapes;
	instances_ = instances;

	const std::filesystem::path shaderDirectory = resourcesDirectory / L"Shaders";
	ComPtr<IDxcBlob> computeShader = shaderCompiler->Compile((shaderDirectory / L"ParticleSimulation.CS.hlsl").wstring(), L"cs_6_0");

	// シェーダーの書き間違いは普通に起きるので、止めずに動かさないだけにする。
	if (computeShader == nullptr) {
		printf("粒を動かすシェーダーを準備できなかったので、粒子を描きません。\n");
		return;
	}

	CreateRootSignature(gpu->GetDevice());
	CreatePipelineState(gpu->GetDevice(), computeShader.Get());
	constantBuffer_.Initialize(gpu->GetDevice(), sizeof(SimulationConstants), kMaxParticleRequestCount);
	isReady_ = true;
}

void ParticleSimulationPass::Execute(const RenderContext& context) {
	ID3D12GraphicsCommandList* list = context.commandList;

	// 形は動かせなくても登録されるので、転送だけは毎フレーム済ませる。
	shapes_->RecordPendingUploads(list);

	if (!isReady_ || requests_->empty()) {
		return;
	}

	// 角度を増やし続けると小数の細かさが足りなくなるので、周期ごとに巻き戻す。
	floatPhase_ = std::fmod(floatPhase_ + context.deltaTime * kFloatRadiansPerSecond, kFloatPhasePeriod);

	// 依頼と同じ並びで粒のバッファを集め、状態の切り替えをまとめて書けるようにする。
	std::vector<ParticleInstance*> instances;
	instances.reserve(requests_->size());
	for (const ParticleRequest& request : *requests_) {
		instances.push_back(&instances_->FindOrCreate(request.instanceId, shapes_->Get(request.shape).targetCount));
	}

	// 前のフレームの命令が終わるとバッファはCOMMONに戻るので、毎フレームCOMMONから書き込める状態にする。
	TransitionStates(list, instances, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	// 設定は粒子ごとに変わらないので、最初に1回だけ書く。
	list->SetPipelineState(pipelineState_.Get());
	list->SetComputeRootSignature(rootSignature_.Get());
	shaderResourceHeap_->Bind(list);

	for (UINT i = 0; i < instances.size(); i++) {
		const ParticleRequest& request = (*requests_)[i];
		ParticleInstance& instance = *instances[i];
		const GpuParticleShape& shape = shapes_->Get(request.shape);

		SimulationConstants constants = {};
		constants.world = request.world;
		constants.deltaTime = context.deltaTime;
		constants.pointCount = instance.pointCount;
		constants.targetCount = shape.targetCount;
		constants.shouldInitialize = instance.shouldInitialize ? 1 : 0;
		constants.initialSettle = request.initialSettle;
		constants.floatDistance = request.floatDistance;
		constants.floatPhase = floatPhase_;
		constants.scatterSeed = request.scatterSeed;
		constants.hasSettleOverride = request.hasSettleOverride ? 1 : 0;
		constants.settleOverride = request.settleOverride;
		constants.gatherPointCount = (std::min)(request.gatherPointCount, kMaxParticleGatherPoints);
		for (uint32_t j = 0; j < constants.gatherPointCount; j++) {
			constants.gatherPoints[j] = request.gatherPoints[j];
		}
		constantBuffer_.Write(i, &constants);
		instance.shouldInitialize = false;

		constantBuffer_.BindCompute(list, kConstantsRootParameterIndex, i);
		list->SetComputeRootDescriptorTable(kStatesRootParameterIndex, shaderResourceHeap_->GetGpuHandle(instance.writeHeapIndex));
		list->SetComputeRootDescriptorTable(kTargetsRootParameterIndex, shape.targetsGpuHandle);

		// 割り切れない分も動かすために切り上げる。
		const UINT threadGroupCount = (instance.pointCount + kParticleThreadGroupSize - 1) / kParticleThreadGroupSize;
		list->Dispatch(threadGroupCount, 1, 1);
	}

	// 描く工程の頂点シェーダーが読む前に、書き込みが終わるのを待たせる。
	TransitionStates(list, instances, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}

void ParticleSimulationPass::TransitionStates(ID3D12GraphicsCommandList* list, const std::vector<ParticleInstance*>& instances, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after) {
	std::vector<D3D12_RESOURCE_BARRIER> barriers(instances.size());
	for (size_t i = 0; i < instances.size(); i++) {
		barriers[i].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barriers[i].Transition.pResource = instances[i]->stateBuffer.Get();
		barriers[i].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barriers[i].Transition.StateBefore = before;
		barriers[i].Transition.StateAfter = after;
	}
	list->ResourceBarrier(static_cast<UINT>(barriers.size()), barriers.data());
}

void ParticleSimulationPass::CreateRootSignature(ID3D12Device* device) {
	const D3D12_DESCRIPTOR_RANGE statesRange = MakeSingleRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0);
	const D3D12_DESCRIPTOR_RANGE targetsRange = MakeSingleRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0);

	// コンピュートシェーダーには見える範囲の区別がないので、どの入口もALLにする。
	D3D12_ROOT_PARAMETER rootParameters[3] = {};

	// 集める場所の一覧はルート定数に入りきらないので、b0は定数バッファで受け取る。
	rootParameters[kConstantsRootParameterIndex] = MakeConstantsParameter(0, D3D12_SHADER_VISIBILITY_ALL);

	// u0で粒のバッファに書き込み、t0で集まる先を読む。
	rootParameters[kStatesRootParameterIndex] = MakeTableParameter(&statesRange, D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[kTargetsRootParameterIndex] = MakeTableParameter(&targetsRange, D3D12_SHADER_VISIBILITY_ALL);

	D3D12_ROOT_SIGNATURE_DESC description = {};
	description.NumParameters = _countof(rootParameters);
	description.pParameters = rootParameters;
	description.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
	rootSignature_ = CreateRootSignatureFromDescription(device, description);
}

void ParticleSimulationPass::CreatePipelineState(ID3D12Device* device, IDxcBlob* computeShader) {
	D3D12_COMPUTE_PIPELINE_STATE_DESC description = {};
	description.pRootSignature = rootSignature_.Get();
	description.CS = { computeShader->GetBufferPointer(), computeShader->GetBufferSize() };

	const HRESULT result = device->CreateComputePipelineState(&description, IID_PPV_ARGS(&pipelineState_));
	KUJATA_ASSERT(SUCCEEDED(result));
}

} // namespace KujataEngine
