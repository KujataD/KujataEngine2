#pragma once

#include "Core/IModule.h"
#include "Core/Math/Vector3.h"
#include "Core/Math/Vector4.h"
#include "Interface/ILineRenderer.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace KujataEngine {

// 中身はLineRendering.cppだけが知っていればよいので、クラスの名前だけを伝える。
class LinePass;

// 1フレームに描ける線の数で、超えた分は描かない。太い線は1本に6頂点を使うので少なめにする。
const uint32_t kMaxThinLineCount = 65536;
const uint32_t kMaxThickLineCount = 16384;

/// <summary>
/// 細い線の頂点1つ分で、Line.VS.hlslのVertexInputと並びを合わせる。
/// </summary>
struct LineVertex {
	Vector3 position;
	Vector4 color;
};

/// <summary>
/// 太い線1本分で、ThickLine.VS.hlslのThickLineと並びを合わせる。
/// </summary>
struct ThickLineSegment {
	Vector3 start;
	float thickness;
	Vector3 end;

	// LineWidthUnitを数にしたもので、0がワールド、1がピクセル。
	uint32_t unit;

	Vector4 color;
};

/// <summary>
/// このフレームに頼まれた線を、描き方ごとに分けて預かる入れ物。
/// </summary>
struct LineBatches {
	std::vector<LineVertex> thinDepthTested;
	std::vector<LineVertex> thinAlwaysVisible;
	std::vector<ThickLineSegment> thickDepthTested;
	std::vector<ThickLineSegment> thickAlwaysVisible;

	/// <summary>
	/// 全部の一覧を空にする。
	/// </summary>
	void Clear();
};

/// <summary>
/// 線を描くように頼む窓口を出し、頼まれた線を描画の工程で描くモジュール。
/// </summary>
class LineRendering : public IModule, public ILineRenderer {
public:
	LineRendering();
	~LineRendering() override;

	std::vector<std::string> GetProvidedServices() override;
	std::vector<std::string> GetRequiredServices() override;
	void Startup(ServiceRegistry& services, FrameLoop& frameLoop) override;

	void SubmitLine(const Vector3& start, const Vector3& end, const Vector4& color, bool depthTest) override;
	void SubmitThickLine(const Vector3& start, const Vector3& end, const Vector4& color, float thickness, LineWidthUnit unit, bool depthTest) override;

private:
	LineBatches batches_;
	std::unique_ptr<LinePass> pass_;
	bool hasWarnedTooManyThinLines_ = false;
	bool hasWarnedTooManyThickLines_ = false;
};

} // namespace KujataEngine
