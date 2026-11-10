#pragma once
#include "AABB.h"
#include "Rect.h"
#include <vector>
#include "Core/Math/MathUtil.h"
#include "Core/Math/Segment.h"

namespace KujataEngine {

// 図形仮置き　後々それぞれのヘッダーを分けます
struct Triangle {
	Vector3 vertices[3];
};

struct Spring {
	Vector3 anchor;           // アンカー
	float naturalLength;      // 自然調
	float stiffness;          // 剛性
	float dampingCoefficient; // 減衰係数
};

struct Ball {
	Vector3 position;
	Vector3 velocity;
	Vector3 acceleration;
	float mass;
	float radius;
	unsigned int color;
};

struct Pendulum {
	Vector3 anchor;
	float length;
	float angle;
	float angularVelocity;
	float angularAcceleration;
};

struct ConicalPendulum {
	Vector3 anchor;
	float length;
	float halfApexAngle;
	float angle;
	float angularVelocity;
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

namespace ShapeUtil {

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
/// 軸が重なっているかどうか
/// </summary>
bool IsOverlappingOnAxis(const OBB& A, const OBB& B, const Vector3& axis);

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

Vector3 Reflect(const Vector3& input, const Vector3& normal);

/// <summary>
/// p1を0、p2を1とした割合tで、p1とp2の間にあるCatmull-Romの曲線上の点を返す。
/// </summary>
Vector3 CatmullRomInterpolation(const Vector3& p0, const Vector3& p1, const Vector3& p2, const Vector3& p3, float t);

/// <summary>
/// 全区間を0から1とした割合tで、制御点を通るCatmull-Romの曲線上の点を返す。
/// </summary>
Vector3 CatmullRomPosition(const std::vector<Vector3>& points, float t);

Segment MakeLimitedSegment(const Vector3& start, const Vector3& end, float maxDistance);
Segment MakeNattoSegment(const Vector3& start, const Vector3& end, float maxDistance, float minDistance);

} // namespace ShapeUtil

} // namespace KujataEngine