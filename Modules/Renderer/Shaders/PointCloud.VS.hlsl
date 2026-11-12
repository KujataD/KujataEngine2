// PointCloudPass.cppのPointCloudConstantsと並びを合わせる。
cbuffer PointCloudConstants : register(b0) {
	float4x4 gView;
	float4x4 gProjection;
	float gPointSize;
	float gIntensity;
	uint gTargetCount;
	float gPadding;
};

// PointCloudPass.cppのPointStateと並びを合わせ、使わない速度も読み飛ばすために置く。
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

StructuredBuffer<PointState> gPoints : register(t0);
StructuredBuffer<PointCloudTarget> gTargets : register(t1);

struct VertexOutput {
	float4 position : SV_POSITION;
	float3 color : COLOR;
};

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

	// 点の色は、同じ番号の集まる先の色にする。
	output.color = gTargets[instanceId % gTargetCount].color;
	return output;
}
