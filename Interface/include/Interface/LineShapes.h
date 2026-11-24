#pragma once

#include "Core/Math/Matrix4x4.h"
#include "Interface/ILineRenderer.h"

#include <cmath>
#include <numbers>

namespace KujataEngine {

// 円と球の円周を、何本の線で描くかの既定値。
const int kDefaultLineShapeSegments = 32;

/// <summary>
/// ローカルのXZ平面にある直径1の円を、ワールド行列で動かして線で描く。円の向きは行列の回転で決まる。
/// </summary>
inline void SubmitLineCircle(ILineRenderer& lines, const Matrix4x4& world, const Vector4& color, int segments = kDefaultLineShapeSegments, bool depthTest = true) {
	// 3本より少ないと、円の形にならない。
	if (segments < 3) {
		segments = 3;
	}

	const float radius = 0.5f;
	const float step = 2.0f * std::numbers::pi_v<float> / static_cast<float>(segments);
	Vector3 previous = Transform({ radius, 0.0f, 0.0f }, world);
	for (int i = 1; i <= segments; i++) {
		const float angle = step * static_cast<float>(i);
		const Vector3 current = Transform({ std::cos(angle) * radius, 0.0f, std::sin(angle) * radius }, world);
		lines.SubmitLine(previous, current, color, depthTest);
		previous = current;
	}
}

/// <summary>
/// 直径1の球を、互いに垂直な3つの円で描き、ワールド行列で動かす。
/// </summary>
inline void SubmitLineSphere(ILineRenderer& lines, const Matrix4x4& world, const Vector4& color, int segments = kDefaultLineShapeSegments, bool depthTest = true) {
	const float quarterTurn = std::numbers::pi_v<float> * 0.5f;
	SubmitLineCircle(lines, world, color, segments, depthTest);
	SubmitLineCircle(lines, MakeRotateXMatrix(quarterTurn) * world, color, segments, depthTest);
	SubmitLineCircle(lines, MakeRotateZMatrix(quarterTurn) * world, color, segments, depthTest);
}

/// <summary>
/// 一辺1で中心が原点の箱の12本の辺を、ワールド行列で動かして描く。MeshRendererの立方体と同じ大きさになる。
/// </summary>
inline void SubmitLineBox(ILineRenderer& lines, const Matrix4x4& world, const Vector4& color, bool depthTest = true) {
	// 番号の1、2、4のビットで、X、Y、Zのどちらの端の角かを表す。
	Vector3 corners[8];
	for (int i = 0; i < 8; i++) {
		const Vector3 local = { (i & 1) ? 0.5f : -0.5f, (i & 2) ? 0.5f : -0.5f, (i & 4) ? 0.5f : -0.5f };
		corners[i] = Transform(local, world);
	}

	// ビットが1つだけ違う角どうしを結ぶと、ちょうど12本の辺になる。
	for (int i = 0; i < 8; i++) {
		for (int bit = 1; bit < 8; bit <<= 1) {
			if ((i & bit) == 0) {
				lines.SubmitLine(corners[i], corners[i | bit], color, depthTest);
			}
		}
	}
}

/// <summary>
/// ローカルのXZ平面にある一辺1で中心が原点の正方形を、X方向とZ方向に指定の数で区切った格子を、ワールド行列で動かして描く。
/// </summary>
inline void SubmitLineGrid(ILineRenderer& lines, const Matrix4x4& world, const Vector4& color, int divisionsX, int divisionsZ, bool depthTest = true) {
	if (divisionsX < 1) {
		divisionsX = 1;
	}
	if (divisionsZ < 1) {
		divisionsZ = 1;
	}

	// 区切りの数より1本多い線で、両端の枠も含めて描く。
	for (int i = 0; i <= divisionsX; i++) {
		const float x = -0.5f + static_cast<float>(i) / static_cast<float>(divisionsX);
		lines.SubmitLine(Transform({ x, 0.0f, -0.5f }, world), Transform({ x, 0.0f, 0.5f }, world), color, depthTest);
	}
	for (int i = 0; i <= divisionsZ; i++) {
		const float z = -0.5f + static_cast<float>(i) / static_cast<float>(divisionsZ);
		lines.SubmitLine(Transform({ -0.5f, 0.0f, z }, world), Transform({ 0.5f, 0.0f, z }, world), color, depthTest);
	}
}

} // namespace KujataEngine
