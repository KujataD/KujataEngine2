// 描く側が頂点シェーダーへ渡す定数バッファと並びを合わせる。
cbuffer TransformData : register(b0) {
	float4x4 gWorldViewProjection;
};

// AssetData.hのMeshVertexと並びを合わせ、法線は受け取るだけで使わない。
struct VertexInput {
	float3 position : POSITION;
	float2 texcoord : TEXCOORD;
	float3 normal : NORMAL;
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
