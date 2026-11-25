#pragma once

#include "Core/Math/Matrix4x4.h"
#include "Core/Math/Vector2.h"
#include "Interface/AssetData.h"

#include <cstdint>

namespace KujataEngine {

/// <summary>
/// スプライト1枚分の四角形を作る。pivotは0から1で、その点が原点に来る。
/// </summary>
MeshData MakeSpriteQuad(const Vector2& size, const Vector2& pivot, bool flipX, bool flipY);

/// <summary>
/// UVに掛ける行列を、拡大、Z回転、移動の順で作る。rotationの単位はラジアン。
/// </summary>
Matrix4x4 MakeSpriteUVTransform(const Vector2& offset, const Vector2& scale, float rotation);

/// <summary>
/// 画像のピクセル数から、スプライトのワールドでの大きさを決める。pixelsPerUnitはワールドの長さ1に入るピクセル数。
/// </summary>
Vector2 ComputeSpriteSize(uint32_t pixelWidth, uint32_t pixelHeight, float pixelsPerUnit);

} // namespace KujataEngine
