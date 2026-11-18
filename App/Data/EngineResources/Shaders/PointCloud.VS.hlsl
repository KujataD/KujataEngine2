// PointCloudPass.cppのPointCloudConstantsと並びを合わせる。
cbuffer PointCloudConstants : register(b0) {
	float4x4 gView;
	float4x4 gProjection;
	float gPointSize;
	float gIntensity;
	uint gTargetCount;
	float gScatteredBrightness;
};

// PointCloudPass.cppのPointStateと並びを合わせ、使わない値も読み飛ばすために置く。
struct PointState {
	float3 position;
	float3 velocity;
	float settleGoal;
	float settleDisplay;
};

// Interface/AssetData.hのPointCloudTargetと並びを合わせる。
struct PointCloudTarget {
	float3 position;
	float3 normal;
	float3 color;
};

StructuredBuffer<PointState> gPoints : register(t0);
StructuredBuffer<PointCloudTarget> gTargets : register(t1);

struct VertexOutput {
	float4 position : SV_POSITION;
	float3 color : COLOR;
};

// 点が消える度合いのしきい値の範囲で、形になる度合いがここを下回った点から消える。
static const float kMinVanishThreshold = 0.15f;
static const float kMaxVanishThreshold = 0.85f;

// しきい値の前後で明るさを切り替える幅で、狭いほど点がぱっと消える。
static const float kVanishFadeWidth = 0.08f;

/// <summary>
/// 整数を混ぜて0以上1未満の小数にする乱数で、PointCloudUpdate.CS.hlslのPcgHashと同じ混ぜ方を使う。
/// </summary>
float HashToUnitFloat(uint value) {
	uint state = value * 747796405u + 2891336453u;
	uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
	uint hash = (word >> 22u) ^ word;
	return float(hash >> 8u) / 16777216.0f;
}

// 板の4つの角で、三角形ストリップの順に並べる。
static const float2 kCorners[4] = {
	float2(-1.0f, -1.0f),
	float2(-1.0f, 1.0f),
	float2(1.0f, -1.0f),
	float2(1.0f, 1.0f),
};

VertexOutput main(uint vertexId : SV_VertexID, uint instanceId : SV_InstanceID) {
	float2 corner = kCorners[vertexId];
	float4 viewPosition = mul(float4(gPoints[instanceId].position, 1.0f), gView);

	// カメラから見た空間で横と縦にずらすので、板はいつもカメラを向く。
	viewPosition.xy += corner * gPointSize;

	VertexOutput output;
	output.position = mul(viewPosition, gProjection);

	// 点ごとに違うしきい値を持たせ、形になる度合いが下がっていくと、ばらばらの順に形でない明るさへ切り替わる。
	const float vanishThreshold = lerp(kMinVanishThreshold, kMaxVanishThreshold, HashToUnitFloat(instanceId));
	const float formedAmount = smoothstep(vanishThreshold - kVanishFadeWidth, vanishThreshold + kVanishFadeWidth, gPoints[instanceId].settleDisplay);

	// 点の色は同じ番号の集まる先の色にし、形でない点は決めた倍率の明るさにする。
	const float brightness = lerp(gScatteredBrightness, 1.0f, formedAmount);
	output.color = gTargets[instanceId % gTargetCount].color * brightness;
	return output;
}
