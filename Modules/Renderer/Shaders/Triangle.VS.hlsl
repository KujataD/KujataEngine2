struct VertexInput {
	float3 position : POSITION;
};

float4 main(VertexInput input) : SV_POSITION {
	return float4(input.position, 1.0f);
}
