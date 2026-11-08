#pragma once

namespace KujataEngine {

/// <summary>
/// 4行4列の行列で、頂点を左から掛ける行ベクトルの形で使う。
/// </summary>
struct Matrix4x4 {
	float m[4][4];
};

/// <summary>
/// 2つの行列を掛け、left、rightの順に変換する行列を返す。
/// </summary>
Matrix4x4 Multiply(const Matrix4x4& left, const Matrix4x4& right);

/// <summary>
/// Y軸まわりの回転行列を作り、角度はラジアンで指定する。
/// </summary>
Matrix4x4 MakeRotationYMatrix(float radians);

/// <summary>
/// 平行移動の行列を作る。
/// </summary>
Matrix4x4 MakeTranslateMatrix(float x, float y, float z);

/// <summary>
/// 透視投影の行列を作り、縦の視野角はラジアンで指定する。
/// </summary>
Matrix4x4 MakePerspectiveFovMatrix(float fovYRadians, float aspectRatio, float nearZ, float farZ);

} // namespace KujataEngine
