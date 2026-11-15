// PointCloudPass.cppのPointCloudConstantsと並びを合わせる。
cbuffer PointCloudConstants : register(b0) {
	float4x4 gView;
	float4x4 gProjection;
	float gPointSize;
	float gIntensity;
	uint gTargetCount;
	float gScatteredBrightness;
};

struct VertexOutput {
	float4 position : SV_POSITION;
	float3 color : COLOR;
};

float4 main(VertexOutput input) : SV_TARGET {
	// 板のどこでも同じ明るさで、点の色を足す。
	return float4(input.color * gIntensity, 1.0f);
}
