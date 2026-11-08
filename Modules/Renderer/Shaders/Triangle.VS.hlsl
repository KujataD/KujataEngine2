cbuffer TransformData : register(b0) {
	float4x4 gWorldViewProjection;
};

struct VertexInput {
	float3 position : POSITION;
};

float4 main(VertexInput input) : SV_POSITION {
	return mul(float4(input.position, 1.0f), gWorldViewProjection);
}
