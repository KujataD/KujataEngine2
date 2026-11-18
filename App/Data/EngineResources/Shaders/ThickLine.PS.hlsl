struct VertexOutput {
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

float4 main(VertexOutput input) : SV_TARGET {
	// 頼まれた色をそのまま出し、アルファで下と混ぜるのはブレンドに任せる。
	return input.color;
}
