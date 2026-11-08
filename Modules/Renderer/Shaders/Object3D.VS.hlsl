cbuffer TransformData : register(b0) {
	float4x4 gWorldViewProjection;
};

struct VertexInput {
	float3 position : POSITION;
	float2 texcoord : TEXCOORD;
};

struct VertexOutput {
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD;
};

VertexOutput main(VertexInput input) {
	VertexOutput output;
	output.position = mul(float4(input.position, 1.0f), gWorldViewProjection);
	output.texcoord = input.texcoord;
	return output;
}
