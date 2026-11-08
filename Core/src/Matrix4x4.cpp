#include "Core/Matrix4x4.h"

#include <cmath>

namespace KujataEngine {

Matrix4x4 Multiply(const Matrix4x4& left, const Matrix4x4& right) {
	Matrix4x4 result = {};
	for (int row = 0; row < 4; row++) {
		for (int column = 0; column < 4; column++) {
			for (int k = 0; k < 4; k++) {
				result.m[row][column] += left.m[row][k] * right.m[k][column];
			}
		}
	}
	return result;
}

Matrix4x4 MakeRotationYMatrix(float radians) {
	const float cosine = std::cos(radians);
	const float sine = std::sin(radians);

	Matrix4x4 result = {};
	result.m[0][0] = cosine;
	result.m[0][2] = -sine;
	result.m[1][1] = 1.0f;
	result.m[2][0] = sine;
	result.m[2][2] = cosine;
	result.m[3][3] = 1.0f;
	return result;
}

Matrix4x4 MakeTranslateMatrix(float x, float y, float z) {
	Matrix4x4 result = {};
	result.m[0][0] = 1.0f;
	result.m[1][1] = 1.0f;
	result.m[2][2] = 1.0f;
	result.m[3][0] = x;
	result.m[3][1] = y;
	result.m[3][2] = z;
	result.m[3][3] = 1.0f;
	return result;
}

Matrix4x4 MakePerspectiveFovMatrix(float fovYRadians, float aspectRatio, float nearZ, float farZ) {
	const float yScale = 1.0f / std::tan(fovYRadians / 2.0f);

	Matrix4x4 result = {};
	result.m[0][0] = yScale / aspectRatio;
	result.m[1][1] = yScale;
	result.m[2][2] = farZ / (farZ - nearZ);
	result.m[2][3] = 1.0f;
	result.m[3][2] = -nearZ * farZ / (farZ - nearZ);
	return result;
}

} // namespace KujataEngine
