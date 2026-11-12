struct VertexOutput {
	float4 position : SV_POSITION;
	float2 corner : TEXCOORD;
};

float4 main(VertexOutput input) : SV_TARGET {
	// 板のどの角かが分かるように、角の位置を色にする。
	float2 color = input.corner * 0.5f + 0.5f;
	return float4(color, 1.0f, 1.0f);
}
