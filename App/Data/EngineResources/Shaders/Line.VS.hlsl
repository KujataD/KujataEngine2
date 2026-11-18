// LinePass.cppのLineConstantsと並びを合わせる。
cbuffer LineConstants : register(b0) {
	float4x4 gViewProjection;
	float4x4 gView;
	float4x4 gProjection;
	float2 gViewportSize;
	float2 gPadding;
};

// Lines.hのLineVertexとLinePass.cppの入力の設定と並びを合わせる。
struct VertexInput {
	float3 position : POSITION;
	float4 color : COLOR;
};

struct VertexOutput {
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

VertexOutput main(VertexInput input) {
	VertexOutput output;
	output.position = mul(float4(input.position, 1.0f), gViewProjection);
	output.color = input.color;
	return output;
}
