Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

// OpaquePass.cppのLightDataと並びを合わせる。
cbuffer LightData : register(b1) {
	float3 gLightDirection;
	float gLightIntensity;
	float3 gLightColor;
	float gPadding1;
	float3 gAmbientColor;
	float gPadding2;
};

struct VertexOutput {
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD;
	float3 normal : NORMAL;
};

float4 main(VertexOutput input) : SV_TARGET {
	float4 textureColor = gTexture.Sample(gSampler, input.texcoord);

	// 頂点の間で補間すると長さが1からずれるので、計算の前に戻す。
	float3 normal = normalize(input.normal);

	// 光と向き合うほど明るく、光に背を向けた面は0にする。
	float lambert = saturate(dot(normal, -gLightDirection));

	float3 lighting = gAmbientColor + gLightColor * gLightIntensity * lambert;
	return float4(textureColor.rgb * lighting, textureColor.a);
}
