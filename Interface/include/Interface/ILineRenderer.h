#pragma once

#include "Core/Math/Vector3.h"
#include "Core/Math/Vector4.h"

namespace KujataEngine {

// ServiceRegistryに登録するときの名前。
const char* const kLineRendererServiceName = "LineRenderer";

/// <summary>
/// 太い線の太さを何で測るか。
/// </summary>
enum class LineWidthUnit {
	// ワールドの長さで、遠くの線ほど細く見える。
	World,

	// 画面のピクセルで、距離に関係なく同じ太さに見える。
	Pixel,
};

/// <summary>
/// 線を描くように頼む窓口で、頼んだフレームにだけ描かれるので、描き続けるには毎フレーム頼む。
/// </summary>
class ILineRenderer {
public:
	virtual ~ILineRenderer() = default;

	/// <summary>
	/// ワールド座標の2点を結ぶ1ピクセルの線を頼む。色はRGBAで、depthTestがtrueなら物の後ろで隠れる。
	/// </summary>
	virtual void SubmitLine(const Vector3& start, const Vector3& end, const Vector4& color, bool depthTest = true) = 0;

	/// <summary>
	/// ワールド座標の2点を結び、カメラに向いた帯で描く太い線を頼む。太さは帯の幅で、単位で測り方を選ぶ。
	/// </summary>
	virtual void SubmitThickLine(const Vector3& start, const Vector3& end, const Vector4& color, float thickness, LineWidthUnit unit, bool depthTest = true) = 0;
};

} // namespace KujataEngine
