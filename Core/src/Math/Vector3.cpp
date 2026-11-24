#include "Core/Math/Vector3.h"

#include <cmath>

namespace KujataEngine {

float Dot(const Vector3& v1, const Vector3& v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z; }
Vector3 Cross(const Vector3& a, const Vector3& b) { return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }
float Length(const Vector3& v) { return std::sqrt(Dot(v, v)); }
Vector3 Normalize(const Vector3& v) {
	float len = Length(v);

	if (len == 0.0f) {
		return Vector3{0.0f, 0.0f, 0.0f};
	}

	return v / len;
}

Vector3 Reflect(const Vector3& input, const Vector3& normal) { return input - normal * (2.0f * Dot(input, normal)); }

Vector3 Project(const Vector3& a, const Vector3& b) {
	float dotAB = Dot(a, b);
	float fieldNormB = std::powf(Length(b), 2);
	float dotNormAB = dotAB / fieldNormB;
	Vector3 result = b * dotNormAB;
	return result;
}

Vector3 Perpendicular(const Vector3& vector) {
	if (vector.x != 0.0f || vector.y != 0.0f) {
		return {-vector.y, vector.x, 0.0f};
	}
	return {0.0f, -vector.z, vector.y};
}

Vector3 Limit(Vector3 v, float max) {
	float length = Length(v);
	if (length > max) {
		Vector3 result = Normalize(v);
		result.x *= max;
		result.y *= max;
		result.z *= max;
		return result;
	}
	return v;
}

} // namespace KujataEngine
