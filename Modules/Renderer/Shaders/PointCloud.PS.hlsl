// PointCloudPass.cppのPointCloudConstantsと並びを合わせる。
cbuffer PointCloudConstants : register(b0) {
	float4x4 gView;
	float4x4 gProjection;
	float gPointSize;
	float gIntensity;
	float2 gPadding;
};

struct VertexOutput {
	float4 position : SV_POSITION;
};

float4 main(VertexOutput input) : SV_TARGET {
	// 板のどこでも同じ明るさの白を足す。
	return float4(gIntensity, gIntensity, gIntensity, 1.0f);
}
