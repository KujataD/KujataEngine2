#pragma once

#include "Core/Math/Vector3.h"
#include "GraphicsKit/ShaderResourceHeap.h"

#include <cstdint>
#include <d3d12.h>
#include <map>
#include <vector>
#include <wrl/client.h>

namespace KujataEngine {

// 粒1つ分の動く状態で、ParticleDraw.VS.hlslとParticleSimulation.CS.hlslのParticleStateと並びを合わせる。
struct ParticleState {
	Vector3 position;
	Vector3 velocity;

	// 形になる度合いの目標と、目標へ少しずつ近づく今の値。
	float settleGoal;
	float settleDisplay;
};
static_assert(sizeof(ParticleState) == 32, "ParticleStateの大きさが変わったらParticleDraw.VS.hlslとParticleSimulation.CS.hlslも直す。");

// ParticleSimulation.CS.hlslのnumthreadsと合わせる。
const UINT kParticleThreadGroupSize = 256;

// 1回のDispatchで動かせる組の数に上限があるので、1つの粒子の粒の数もここまでにする。
const UINT kMaxPointCountPerParticle = D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION * kParticleThreadGroupSize;

/// <summary>
/// 描く物1つ分の、粒ごとの位置と速度と形になる度合いを持つバッファと名札。
/// </summary>
struct ParticleInstance {
	Microsoft::WRL::ComPtr<ID3D12Resource> stateBuffer;

	// 棚の中の場所で、読む名札と書く名札の2つを持つ。
	UINT readHeapIndex = 0;
	UINT writeHeapIndex = 0;

	UINT pointCount = 0;

	// 作ったばかりのバッファには何も入っていないので、最初の更新で粒を散った位置に置く。
	bool shouldInitialize = true;

	// このフレームで依頼が来たかどうかで、来なくなった物を片付けるのに使う。
	bool isUsedThisFrame = false;
};

/// <summary>
/// 依頼の番号ごとに粒の状態のバッファを持ち続け、依頼が来なくなった物を片付ける。
/// </summary>
class ParticleInstances {
public:
	void Initialize(ID3D12Device* device, ShaderResourceHeap* shaderResourceHeap);

	/// <summary>
	/// 番号に対応する粒のバッファを返し、初めての番号なら指定の数の粒で作る。返した物はこのフレームで使った印が付く。
	/// </summary>
	ParticleInstance& FindOrCreate(uint64_t instanceId, UINT pointCount);

	/// <summary>
	/// 番号に対応する粒のバッファを返し、なければnullptrを返す。
	/// </summary>
	const ParticleInstance* Find(uint64_t instanceId) const;

	/// <summary>
	/// このフレームで使わなかった粒のバッファを捨て、棚の場所を使い回せるようにする。
	/// </summary>
	void RemoveUnused();

private:
	/// <summary>
	/// 棚の場所を1つ確保し、捨てた物の場所があればそれを使い回す。
	/// </summary>
	UINT AllocateHeapIndex();

	ID3D12Device* device_ = nullptr;
	ShaderResourceHeap* shaderResourceHeap_ = nullptr;

	// 依頼の番号から、その物の粒のバッファを引く。
	std::map<uint64_t, ParticleInstance> instances_;

	// 捨てた粒のバッファが使っていた棚の場所。
	std::vector<UINT> reusableHeapIndices_;

	bool hasWarnedTooManyPoints_ = false;
};

} // namespace KujataEngine
