// PointCloudPass.cppのPointUpdateConstantsと並びを合わせる。
cbuffer UpdateConstants : register(b0) {
	float3 gBoxCenter;
	float gDeltaTime;
	float3 gBoxHalfSize;
	uint gPointCount;
	uint gFrameIndex;
};

// PointCloudPass.cppのPointと並びを合わせる。
struct Point {
	float3 position;
	float3 velocity;
};

RWStructuredBuffer<Point> gPoints : register(u0);

// 速度を揺らす強さで、単位はメートル毎秒毎秒。
static const float kJitterAcceleration = 4.0f;

// 1秒あたりに速度を減らす割合。
static const float kDampingPerSecond = 2.0f;

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

	// 点の番号とフレームの番号を混ぜて、点ごとにもフレームごとにも違う乱数にする。
	const uint seedX = PcgHash(index ^ PcgHash(gFrameIndex));
	const uint seedY = PcgHash(seedX);
	const uint seedZ = PcgHash(seedY);
	const float3 jitter = float3(ToUnitFloat(seedX), ToUnitFloat(seedY), ToUnitFloat(seedZ)) * 2.0f - 1.0f;

	Point current = gPoints[index];
	current.velocity += jitter * kJitterAcceleration * gDeltaTime;
	current.velocity -= current.velocity * kDampingPerSecond * gDeltaTime;
	current.position += current.velocity * gDeltaTime;

	// 箱から出た点は、反対側の面から入り直させる。
	const float3 boxSize = gBoxHalfSize * 2.0f;
	const float3 offsetFromBoxMin = current.position - (gBoxCenter - gBoxHalfSize);
	current.position = gBoxCenter - gBoxHalfSize + frac(offsetFromBoxMin / boxSize) * boxSize;

	gPoints[index] = current;
}
