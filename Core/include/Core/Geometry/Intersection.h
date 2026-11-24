#pragma once

#include "Core/Geometry/AABB.h"
#include "Core/Geometry/Shapes.h"
#include "Core/Geometry/Segment.h"

namespace KujataEngine {

/// <summary>
/// 押し出しと速度の反射に使う接触の情報。
/// </summary>
struct Contact {
	// 第1引数から第2引数へ向かう長さ1の向きで、第1の形は逆向きに、第2の形はこの向きに押し出す。
	Vector3 normal{};

	// 0以上のめり込みの量。
	float depth = 0.0f;

	// ワールド座標でのおおよその接触点。
	Vector3 point{};
};

bool IsCollision(const Sphere& sphere, const Plane& plane);

bool IsCollision(const Segment& line, const Plane& plane);

bool IsCollision(const Segment& segment, const Triangle& triangle);

bool IsCollision(const AABB& aabb1, const AABB& aabb2);

bool IsCollision(const AABB& aabb, const Sphere& sphere);

bool IsCollision(const AABB& aabb, const Vector3& point);

bool IsCollision(const AABB& aabb, const Segment& segment);

bool IsCollision(const AABB& aabb, const Line& line);

bool IsCollision(const AABB& aabb, const Ray& ray);

bool IsCollision(const OBB& obb, const Sphere& sphere);

bool IsCollision(const OBB& obb, const Segment& segment);

bool IsCollision(const OBB& obb, const Line& line);

bool IsCollision(const OBB& obb, const Ray& ray);

bool IsCollision(const OBB& obb1, const OBB& obb2);

bool IsCollision(const Sphere& a, const Sphere& b);

bool IsCollision(const Capsule& capsule, const Sphere& sphere);

bool IsCollision(const Capsule& a, const Capsule& b);

bool IsCollision(const Capsule& capsule, const OBB& obb);

/// <summary>
/// 線分の始点から最初に当たる位置の割合を0から1で返し、始点が形の中なら0で当たったとする。
/// </summary>
bool RaycastSegment(const Segment& segment, const Sphere& sphere, float& outT);
bool RaycastSegment(const Segment& segment, const AABB& aabb, float& outT);
bool RaycastSegment(const Segment& segment, const OBB& obb, float& outT);
bool RaycastSegment(const Segment& segment, const Capsule& capsule, float& outT);

/// <summary>
/// 交差していれば、第1引数から第2引数へ向かう法線で接触情報を埋めてtrueを返す。
/// </summary>
bool ComputeContact(const Sphere& a, const Sphere& b, Contact& out);
bool ComputeContact(const Sphere& sphere, const OBB& obb, Contact& out);
bool ComputeContact(const OBB& a, const OBB& b, Contact& out);
// normal は capsule から相手へ向かう分離方向(単位)。
bool ComputeContact(const Capsule& capsule, const Sphere& sphere, Contact& out);
bool ComputeContact(const Capsule& a, const Capsule& b, Contact& out);
bool ComputeContact(const Capsule& capsule, const OBB& obb, Contact& out);

} // namespace KujataEngine