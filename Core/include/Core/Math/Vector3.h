#pragma once
#include <assert.h>
#include <cmath>
#include <numbers>

namespace KujataEngine {

class Vector3 {
public:
	float x;
	float y;
	float z;

	Vector3 operator+(const Vector3& v) const { return {x + v.x, y + v.y, z + v.z}; }
	Vector3 operator-(const Vector3& v) const { return {x - v.x, y - v.y, z - v.z}; }
	Vector3 operator-() const { return {-x, -y, -z}; }

	void operator+=(const Vector3& v) {
		x += v.x;
		y += v.y;
		z += v.z;
	}
	void operator-=(const Vector3& v) {
		x -= v.x;
		y -= v.y;
		z -= v.z;
	}
	void operator*=(float scalar) {
		x *= scalar;
		y *= scalar;
		z *= scalar;
	}
	void operator/=(float scalar) {
		x /= scalar;
		y /= scalar;
		z /= scalar;
	}
};

inline Vector3 operator+(Vector3 v, float f) { return {v.x + f, v.y + f, v.z + f}; }
inline Vector3 operator-(Vector3 v, float f) { return {v.x - f, v.y - f, v.z - f}; }
inline Vector3 operator*(Vector3 v, float f) { return {v.x * f, v.y * f, v.z * f}; }
inline Vector3 operator/(Vector3 v, float f) { return {v.x / f, v.y / f, v.z / f}; }
inline Vector3 operator+(float f, Vector3 v) { return {v.x + f, v.y + f, v.z + f}; }
inline Vector3 operator-(float f, Vector3 v) { return {v.x - f, v.y - f, v.z - f}; }
inline Vector3 operator*(float f, Vector3 v) { return {v.x * f, v.y * f, v.z * f}; }

float Dot(const Vector3& v1, const Vector3& v2);
Vector3 Cross(const Vector3& a, const Vector3& b);
float Length(const Vector3& v);
Vector3 Normalize(const Vector3& v);
Vector3 Reflect(const Vector3& input, const Vector3& normal);

// aをbの向きへ射影したベクトルを返す。
Vector3 Project(const Vector3& a, const Vector3& b);

// 垂直なベクトルを1つ返す。
Vector3 Perpendicular(const Vector3& vector);

// 長さがmaxを超えていたら、向きはそのままでmaxに縮める。
Vector3 Limit(Vector3 v, float max);

} // namespace KujataEngine
