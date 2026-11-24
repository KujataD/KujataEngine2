#pragma once
#include "Core/Math/Vector3.h"
#include <algorithm>
#include <assert.h>
#include <cmath>
#include <numbers>

namespace KujataEngine {
class Matrix4x4 {
public:
	float m[4][4];

	Matrix4x4 operator+(const Matrix4x4& m) const;
	Matrix4x4 operator-(const Matrix4x4& m) const;
	Matrix4x4 operator*(const Matrix4x4& m) const;
	Matrix4x4 operator/(float scalar) const;
};

Matrix4x4 MakeAffineMatrixOrientations(const Vector3 orientations[3], const Vector3& translate);

Matrix4x4 Inverse(const Matrix4x4& m);
Matrix4x4 Transpose(const Matrix4x4& m);
Matrix4x4 MakeIdentity();

Matrix4x4 MakeTranslateMatrix(const Vector3& translate);
Matrix4x4 MakeScaleMatrix(const Vector3& scale);
Matrix4x4 MakeRotateXMatrix(float radian);
Matrix4x4 MakeRotateYMatrix(float radian);
Matrix4x4 MakeRotateZMatrix(float radian);
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);
Matrix4x4 MakeRotateMatrix( const Vector3& rotate);

// 透視投影行列
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);

// 正射影行列
Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);

// ビューポート変換行列
Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);

// 点として変換し、wで割る。
Vector3 Transform(const Vector3& v, const Matrix4x4& m);

// 向きとして変換し、平行移動を掛けない。
Vector3 TransformNormal(const Vector3& v, const Matrix4x4& m);

// ワールドの位置を、ビューポートの中の画面の位置にする。
Vector3 ProjectToScreen(const Vector3& worldPosition, float viewportX, float viewportY, float viewportWidth, float viewportHeight, const Matrix4x4& matView, const Matrix4x4& matProjection);

} // namespace KujataEngine
