#include "SpriteQuad.h"

namespace KujataEngine {

MeshData MakeSpriteQuad(const Vector2& size, const Vector2& pivot, bool flipX, bool flipY) {
	// pivotが原点に来るように、uとvが0の側の端をpivotの分だけずらす。
	const float xAtU0 = pivot.x * size.x;
	const float xAtU1 = -(1.0f - pivot.x) * size.x;
	const float yAtV0 = pivot.y * size.y;
	const float yAtV1 = -(1.0f - pivot.y) * size.y;

	const float u0 = flipX ? 1.0f : 0.0f;
	const float u1 = flipX ? 0.0f : 1.0f;
	const float v0 = flipY ? 1.0f : 0.0f;
	const float v1 = flipY ? 0.0f : 1.0f;

	const Vector3 normal = {0.0f, 0.0f, 1.0f};

	MeshData quad;
	quad.vertices = {
	    {{xAtU0, yAtV0, 0.0f}, {u0, v0}, normal},
	    {{xAtU1, yAtV0, 0.0f}, {u1, v0}, normal},
	    {{xAtU0, yAtV1, 0.0f}, {u0, v1}, normal},
	    {{xAtU1, yAtV1, 0.0f}, {u1, v1}, normal},
	};
	quad.indices = {0, 1, 2, 2, 1, 3};
	return quad;
}

Matrix4x4 MakeSpriteUVTransform(const Vector2& offset, const Vector2& scale, float rotation) {
	Matrix4x4 uvTransform = MakeScaleMatrix({scale.x, scale.y, 1.0f});
	uvTransform = uvTransform * MakeRotateZMatrix(rotation);
	uvTransform = uvTransform * MakeTranslateMatrix({offset.x, offset.y, 0.0f});
	return uvTransform;
}

Vector2 ComputeSpriteSize(uint32_t pixelWidth, uint32_t pixelHeight, float pixelsPerUnit) {
	return {static_cast<float>(pixelWidth) / pixelsPerUnit, static_cast<float>(pixelHeight) / pixelsPerUnit};
}

} // namespace KujataEngine
