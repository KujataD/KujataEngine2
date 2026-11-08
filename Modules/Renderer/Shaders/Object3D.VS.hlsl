cbuffer TransformData : register(b0) {
	float4x4 gWorldViewProjection;
};

struct VertexInput {
	float3 position : POSITION;
	float3 color : COLOR;
};

struct VertexOutput {
	float4 position : SV_POSITION;
	float3 color : COLOR;
};

VertexOutput main(VertexInput input) {
	VertexOutput output;
	output.position = mul(float4(input.position, 1.0f), gWorldViewProjection);
	output.color = input.color;
	return output;
}
