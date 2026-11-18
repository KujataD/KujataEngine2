// LinePass.cppのLineConstantsと並びを合わせる。
cbuffer LineConstants : register(b0) {
	float4x4 gViewProjection;
	float4x4 gView;
	float4x4 gProjection;
	float2 gViewportSize;
	float2 gPadding;
};

// Lines.hのThickLineSegmentと並びを合わせる。
struct ThickLine {
	float3 start;
	float thickness;
	float3 end;
	uint unit;
	float4 color;
};

StructuredBuffer<ThickLine> gLines : register(t0);

struct VertexOutput {
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

// Lines.hのLineWidthUnitを数にしたもの。
static const uint kWorldUnit = 0;

// 板の6頂点の、xは始点(0)か終点(1)か、yは線のどちら側(-1か1)へ広げるか。
static const float2 kCorners[6] = {
	float2(0.0f, -1.0f),
	float2(0.0f, 1.0f),
	float2(1.0f, -1.0f),
	float2(1.0f, -1.0f),
	float2(0.0f, 1.0f),
	float2(1.0f, 1.0f),
};

// 向きが求まらないほど短いときに、代わりに使う長さの2乗のしきい値。
static const float kMinLengthSquared = 1e-12f;

VertexOutput main(uint vertexId : SV_VertexID) {
	const ThickLine segment = gLines[vertexId / 6];
	const float2 corner = kCorners[vertexId % 6];
	const float halfThickness = segment.thickness * 0.5f;

	VertexOutput output;
	output.color = segment.color;

	if (segment.unit == kWorldUnit) {
		// カメラから見た空間では目が原点なので、線の向きと、目から点への向きの両方に垂直な向きへ広げる。
		const float3 startView = mul(float4(segment.start, 1.0f), gView).xyz;
		const float3 endView = mul(float4(segment.end, 1.0f), gView).xyz;
		const float3 positionView = lerp(startView, endView, corner.x);

		float3 side = cross(endView - startView, positionView);
		const float sideLengthSquared = dot(side, side);
		side = (sideLengthSquared > kMinLengthSquared) ? side * rsqrt(sideLengthSquared) : float3(1.0f, 0.0f, 0.0f);

		output.position = mul(float4(positionView + side * (halfThickness * corner.y), 1.0f), gProjection);
		return output;
	}

	// ピクセルで測るときは、画面上の線の向きに垂直な向きへ広げる。目より後ろの点は扱わない。
	const float4 startClip = mul(float4(segment.start, 1.0f), gViewProjection);
	const float4 endClip = mul(float4(segment.end, 1.0f), gViewProjection);
	const float2 startScreen = startClip.xy / startClip.w * gViewportSize;
	const float2 endScreen = endClip.xy / endClip.w * gViewportSize;

	float2 direction = endScreen - startScreen;
	const float directionLengthSquared = dot(direction, direction);
	direction = (directionLengthSquared > kMinLengthSquared) ? direction * rsqrt(directionLengthSquared) : float2(1.0f, 0.0f);
	const float2 normal = float2(-direction.y, direction.x);

	// 画面の-1から1は画面の幅の分なので、ピクセルを2/画面の大きさ倍し、透視の割り算を打ち消すためにwを掛ける。
	float4 positionClip = lerp(startClip, endClip, corner.x);
	positionClip.xy += normal * (halfThickness * corner.y) * (2.0f / gViewportSize) * positionClip.w;
	output.position = positionClip;
	return output;
}
