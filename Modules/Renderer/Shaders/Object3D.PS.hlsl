Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct VertexOutput {
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD;
};

float4 main(VertexOutput input) : SV_TARGET {
	return gTexture.Sample(gSampler, input.texcoord);
}
