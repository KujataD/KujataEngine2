// PointCloudPass.cppのPointUpdateConstantsと並びを合わせる。
cbuffer UpdateConstants : register(b0) {
	float4x4 gWorld;
	float gDeltaTime;
	uint gPointCount;
	uint gTargetCount;
	uint gFrameIndex;
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

// 速度を揺らす強さで、単位はメートル毎秒毎秒。
static const float kJitterAcceleration = 4.0f;

// 1秒あたりに速度を減らす割合。
static const float kDampingPerSecond = 2.0f;

// 点が集まる先から離れられる距離。
static const float kMaxWanderDistance = 0.05f;

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

	// 作ったばかりのバッファには何も入っていないので、集まる先から始める。
	if (gShouldInitialize != 0) {
		current.position = targetPosition;
		current.velocity = float3(0.0f, 0.0f, 0.0f);
	}

	// 点の番号とフレームの番号を混ぜて、点ごとにもフレームごとにも違う乱数にする。
	const uint seedX = PcgHash(index ^ PcgHash(gFrameIndex));
	const uint seedY = PcgHash(seedX);
	const uint seedZ = PcgHash(seedY);
	const float3 jitter = float3(ToUnitFloat(seedX), ToUnitFloat(seedY), ToUnitFloat(seedZ)) * 2.0f - 1.0f;

	current.velocity += jitter * kJitterAcceleration * gDeltaTime;
	current.velocity -= current.velocity * kDampingPerSecond * gDeltaTime;
	current.position += current.velocity * gDeltaTime;

	// 集まる先から離れすぎた点は、決めた距離まで戻す。
	const float3 offsetFromTarget = current.position - targetPosition;
	const float distanceFromTarget = length(offsetFromTarget);
	if (distanceFromTarget > kMaxWanderDistance) {
		current.position = targetPosition + offsetFromTarget * (kMaxWanderDistance / distanceFromTarget);
	}

	gPoints[index] = current;
}
