// PointCloudPass.cppのPointUpdateConstantsと並びを合わせる。
cbuffer UpdateConstants : register(b0) {
	float4x4 gWorld;
	float gDeltaTime;
	uint gPointCount;
	uint gTargetCount;
	uint gShouldInitialize;
};

// PointCloudPass.cppのPointStateと並びを合わせる。
struct PointState {
	float3 position;
	float3 velocity;
};

// Interface/AssetData.hのPointCloudTargetと並びを合わせる。
struct PointCloudTarget {
	float3 position;
	float3 normal;
	float3 color;
};

RWStructuredBuffer<PointState> gPoints : register(u0);
StructuredBuffer<PointCloudTarget> gTargets : register(t0);

// 集まる先へ引くばねの強さで、大きいほど速く集まる。
static const float kSpringStiffness = 30.0f;

// 速度に比例して止める強さで、小さいほど集まる先を行き過ぎて揺れる。
static const float kSpringDamping = 6.0f;

// 1回に進める秒数の上限で、長いフレームでばねが発散するのを防ぐ。
static const float kMaxDeltaTime = 1.0f / 30.0f;

// 最初に点を散らす範囲で、集まる先からの各軸の最大の距離。
static const float kScatterDistance = 2.0f;

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
/// 点の番号から、その点がいつも同じになる各軸が-1から1の乱数を作る。
/// </summary>
float3 MakeScatterOffset(uint index) {
	const uint seedX = PcgHash(index);
	const uint seedY = PcgHash(seedX);
	const uint seedZ = PcgHash(seedY);
	return float3(ToUnitFloat(seedX), ToUnitFloat(seedY), ToUnitFloat(seedZ)) * 2.0f - 1.0f;
}

// PointCloudPass.cppのkThreadGroupSizeと合わせる。
[numthreads(256, 1, 1)]
void main(uint3 threadId : SV_DispatchThreadID) {
	const uint index = threadId.x;

	// 最後の組には点の数を超えるスレッドがあるので、範囲の外へ書き込まない。
	if (index >= gPointCount) {
		return;
	}

	// 点と集まる先は同じ番号で対応させ、集まる先の方が少なければ頭から繰り返す。
	const PointCloudTarget target = gTargets[index % gTargetCount];
	const float3 targetPosition = mul(float4(target.position, 1.0f), gWorld).xyz;

	PointState current = gPoints[index];

	// 作ったばかりのバッファには何も入っていないので、集まる先のまわりに散らして始める。
	if (gShouldInitialize != 0) {
		current.position = targetPosition + MakeScatterOffset(index) * kScatterDistance;
		current.velocity = float3(0.0f, 0.0f, 0.0f);
	}

	// 速度を先に更新してから位置に使うと、同じ刻みでもばねが発散しにくい。
	const float deltaTime = min(gDeltaTime, kMaxDeltaTime);
	const float3 acceleration = (targetPosition - current.position) * kSpringStiffness - current.velocity * kSpringDamping;
	current.velocity += acceleration * deltaTime;
	current.position += current.velocity * deltaTime;

	gPoints[index] = current;
}
