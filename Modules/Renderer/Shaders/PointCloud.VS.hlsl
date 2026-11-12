// PointCloudPass.cppのPointCloudConstantsと並びを合わせる。
cbuffer CameraData : register(b0) {
	float4x4 gView;
	float4x4 gProjection;
	float gPointSize;
	float3 gPadding;
};

// PointCloudPass.cppのPointと並びを合わせる。
struct Point {
	float3 position;
};

StructuredBuffer<Point> gPoints : register(t0);

struct VertexOutput {
	float4 position : SV_POSITION;
	float2 corner : TEXCOORD;
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
	output.corner = corner;
	return output;
}
