Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

// 描く側がピクセルシェーダーへ渡す定数バッファと並びを合わせる。
cbuffer MaterialData : register(b0) {
	float4 gColor;
	float4x4 gUVTransform;
};

struct VertexOutput {
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD;
};

float4 main(VertexOutput input) : SV_TARGET {
	float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gUVTransform);

	// ライトは通さず、画像の色に色を掛けるだけにする。
	return gTexture.Sample(gSampler, transformedUV.xy) * gColor;
}
