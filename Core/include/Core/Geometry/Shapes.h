#pragma once

#include "Core/Math/Vector3.h"

namespace KujataEngine {

struct Triangle {
	Vector3 vertices[3];
};

struct Ray {
	Vector3 origin; // 始点
	Vector3 diff;   // 終点の差分ベクトル
};

struct Line {
	Vector3 origin; // 始点
	Vector3 diff;   // 終点の差分ベクトル
};

struct Sphere {
	Vector3 center;
	float radius;
};

// カプセル。spine(線分 p0-p1)から半径radius膨らませた形状。p0==p1のとき球と等価。
struct Capsule {
	Vector3 p0;    // spine端点(片側の球中心)
	Vector3 p1;    // spine端点(反対側の球中心)
	float radius;
};

struct Plane {
	Vector3 normal;
	float distance;
};

} // namespace KujataEngine
