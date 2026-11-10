#pragma once

#include "Core/Math/Matrix4x4.h"

namespace KujataEngine {

/// <summary>
/// 描くときのカメラの情報で、画面の縦横比は描く側が決める。
/// </summary>
struct CameraData {
	Matrix4x4 view;
	float fovYRadians;
	float nearZ;
	float farZ;
};

/// <summary>
/// Sceneが描きたい物を描画の担当に頼むための窓口。
/// </summary>
class IRenderQueue {
public:
	virtual ~IRenderQueue() = default;

	/// <summary>
	/// このフレームで使うカメラを指定する。
	/// </summary>
	virtual void SetCamera(const CameraData& camera) = 0;

	/// <summary>
	/// 立方体を指定のワールド行列で描くように頼む。
	/// </summary>
	virtual void SubmitCube(const Matrix4x4& world) = 0;
};

} // namespace KujataEngine
