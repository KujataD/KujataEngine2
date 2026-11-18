// OpaquePass.cppのTransformDataと並びを合わせる。
cbuffer TransformData : register(b0) {
	float4x4 gWorldViewProjection;
	float4x4 gWorldInverseTranspose;
};

// AssetData.hのMeshVertexとPipelineStateの入力の設定と並びを合わせる。
struct VertexInput {
	float3 position : POSITION;
	float2 texcoord : TEXCOORD;
	float3 normal : NORMAL;
};

struct VertexOutput {
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD;
	float3 normal : NORMAL;
};

VertexOutput main(VertexInput input) {
	VertexOutput output;
	output.position = mul(float4(input.position, 1.0f), gWorldViewProjection);
	output.texcoord = input.texcoord;
	// 拡大が軸ごとに違っても面に垂直なままになるように、逆転置行列で法線を回す。
	output.normal = mul(input.normal, (float3x3)gWorldInverseTranspose);
	return output;
}
