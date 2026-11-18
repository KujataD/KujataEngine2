// Interface/ParticleData.hのParticleGatherPointと並びを合わせる。
struct GatherPoint {
	float3 position;
	float radius;
};

// Interface/ParticleData.hのkMaxParticleGatherPointsと合わせる。
static const uint kMaxGatherPoints = 16;

// ParticleSimulationPass.cppのSimulationConstantsと並びを合わせる。
cbuffer SimulationConstants : register(b0) {
	float4x4 gWorld;
	float gDeltaTime;
	uint gPointCount;
	uint gTargetCount;
	uint gShouldInitialize;
	float gInitialSettle;
	uint gGatherPointCount;
	float gFloatDistance;
	float gFloatPhase;
	uint gHasSettleOverride;
	float gSettleOverride;
	uint gScatterSeed;
	float gPadding;
	GatherPoint gGatherPoints[kMaxGatherPoints];
};

// ParticleInstances.hのParticleStateと並びを合わせる。
struct ParticleState {
	float3 position;
	float3 velocity;
	float settleGoal;
	float settleDisplay;
};

// Interface/ParticleData.hのParticleTargetと並びを合わせる。
struct ParticleTarget {
	float3 position;
	float3 normal;
	float3 color;
};

RWStructuredBuffer<ParticleState> gPoints : register(u0);
StructuredBuffer<ParticleTarget> gTargets : register(t0);

// 引き寄せる先へ引くばねの強さで、大きいほど速く集まる。
static const float kSpringStiffness = 30.0f;

// 速度に比例して止める強さで、ばねの強さの平方根の2倍より大きくして、引き寄せる先を行き過ぎて戻る動きを出さない。
static const float kSpringDamping = 11.0f;

// 1回に進める秒数の上限で、長いフレームでばねが発散するのを防ぐ。
static const float kMaxDeltaTime = 1.0f / 30.0f;

// 形になっていない粒が散る範囲で、集まる先からの各軸の最大の距離。
static const float kScatterDistance = 2.0f;

// 形になる度合いが1秒で目標へ近づく割合。
static const float kSettleSpeedPerSecond = 3.0f;

// 粒ごとに近づく速さへ掛ける倍率の最小と最大で、差が大きいほど崩れるタイミングがばらける。
static const float kMinSettleSpeedRatio = 0.25f;
static const float kMaxSettleSpeedRatio = 2.0f;

// 集める場所の半径のうち、この割合より内側の粒は完全に形になる。
static const float kFullGatherRadiusRatio = 0.5f;

/// <summary>
/// 整数を混ぜて、偏りの少ない別の整数にするPCGハッシュ。
/// </summary>
uint PcgHash(uint value) {
	uint state = value * 747796405u + 2891336453u;
	uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
	return (word >> 22u) ^ word;
}

/// <summary>
/// ハッシュの上位24ビットを、0以上1未満の小数にする。
/// </summary>
float ToUnitFloat(uint hash) {
	return float(hash >> 8u) / 16777216.0f;
}

/// <summary>
/// 粒の番号と散り方の種から、種を変えるまでは同じになる各軸が-1から1の乱数を作る。
/// </summary>
float3 MakeScatterOffset(uint index) {
	const uint seedX = PcgHash(index ^ PcgHash(gScatterSeed));
	const uint seedY = PcgHash(seedX);
	const uint seedZ = PcgHash(seedY);
	return float3(ToUnitFloat(seedX), ToUnitFloat(seedY), ToUnitFloat(seedZ)) * 2.0f - 1.0f;
}

/// <summary>
/// 粒ごとに違う拍子でゆっくり行き来する、各軸が-1から1の揺れを作る。
/// </summary>
float3 MakeFloatOffset(uint index) {
	// 散る位置と同じ乱数にならないように、番号をずらしてから混ぜる。
	const uint seedX = PcgHash(index + 0x9E3779B9u);
	const uint seedY = PcgHash(seedX);
	const uint seedZ = PcgHash(seedY);
	const float3 startAngle = float3(ToUnitFloat(seedX), ToUnitFloat(seedY), ToUnitFloat(seedZ)) * 6.2831853f;

	// 軸ごとに速さを変えて同じ道を回らないようにし、倍率はParticleSimulationPass.cppのkFloatPhasePeriodに合わせて0.25の倍数にする。
	const float3 speedRatio = float3(1.0f, 1.25f, 0.75f);
	return sin(gFloatPhase * speedRatio + startAngle);
}

/// <summary>
/// 集まる先が集める場所にどれだけ近いかを、いちばん近い場所について0から1で返す。
/// </summary>
float ComputeGatherAmount(float3 targetPosition) {
	float gatherAmount = 0.0f;
	for (uint i = 0; i < gGatherPointCount; i++) {
		const GatherPoint gatherPoint = gGatherPoints[i];
		const float distanceToGatherPoint = length(targetPosition - gatherPoint.position);

		// 半径の内側ほど1に近づけ、ふちでなめらかに0にする。
		const float amount = 1.0f - smoothstep(gatherPoint.radius * kFullGatherRadiusRatio, gatherPoint.radius, distanceToGatherPoint);
		gatherAmount = max(gatherAmount, amount);
	}
	return gatherAmount;
}

// ParticleInstances.hのkParticleThreadGroupSizeと合わせる。
[numthreads(256, 1, 1)]
void main(uint3 threadId : SV_DispatchThreadID) {
	const uint index = threadId.x;

	// 最後の組には粒の数を超えるスレッドがあるので、範囲の外へ書き込まない。
	if (index >= gPointCount) {
		return;
	}

	// 粒と集まる先は同じ番号で対応させ、集まる先の方が少なければ頭から繰り返す。
	const ParticleTarget target = gTargets[index % gTargetCount];
	const float3 targetPosition = mul(float4(target.position, 1.0f), gWorld).xyz;
	const float3 scatterPosition = targetPosition + MakeScatterOffset(index) * kScatterDistance;

	ParticleState current = gPoints[index];

	// 作ったばかりのバッファには何も入っていないので、形になる度合いを最初の値にする。
	if (gShouldInitialize != 0) {
		current.settleGoal = gInitialSettle;
		current.settleDisplay = gInitialSettle;
	}

	// 全部を崩すときや形に戻すときは、目標をまとめて書き換える。
	if (gHasSettleOverride != 0) {
		current.settleGoal = gSettleOverride;
	}

	// 集める場所では目標を上げるだけなので、集める場所が離れた後も形になったままになる。
	current.settleGoal = max(current.settleGoal, ComputeGatherAmount(targetPosition));

	// 粒ごとに近づく速さを変えて、早く崩れる粒と遅く崩れる粒が混ざるようにする。
	const float speedRandom = ToUnitFloat(PcgHash(index + 0x85EBCA6Bu + gScatterSeed * 0xC2B2AE35u));
	const float settleSpeed = kSettleSpeedPerSecond * lerp(kMinSettleSpeedRatio, kMaxSettleSpeedRatio, speedRandom);

	// 今の値を目標へ少しずつ近づけ、粒がいっせいに飛ばないようにする。
	const float deltaTime = min(gDeltaTime, kMaxDeltaTime);
	current.settleDisplay += (current.settleGoal - current.settleDisplay) * min(settleSpeed * deltaTime, 1.0f);

	// 引き寄せる先を、散った位置と集まる先の間で、形になる度合いに合わせて動かす。
	const float3 restPosition = lerp(scatterPosition, targetPosition, smoothstep(0.0f, 1.0f, current.settleDisplay));

	// 引き寄せる先そのものを揺らすと、ばねが追いかけて粒がふわふわ動き続ける。
	const float3 attractPosition = restPosition + MakeFloatOffset(index) * gFloatDistance;

	// 作ったばかりの粒は、引き寄せる先に止まった状態から始める。
	if (gShouldInitialize != 0) {
		current.position = attractPosition;
		current.velocity = float3(0.0f, 0.0f, 0.0f);
	}

	// 速度を先に更新してから位置に使うと、同じ刻みでもばねが発散しにくい。
	const float3 acceleration = (attractPosition - current.position) * kSpringStiffness - current.velocity * kSpringDamping;
	current.velocity += acceleration * deltaTime;
	current.position += current.velocity * deltaTime;

	gPoints[index] = current;
}
