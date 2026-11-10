#pragma once

#include "Core/Math/Matrix4x4.h"
#include "Interface/AssetHandle.h"

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
/// 描きたい物があるコンポーネントが、描画の担当に描く物を頼むための窓口。
/// </summary>
class IRenderQueue {
public:
	virtual ~IRenderQueue() = default;

	/// <summary>
	/// このフレームで使うカメラを指定する。
	/// </summary>
	virtual void SetCamera(const CameraData& camera) = 0;

	/// <summary>
	/// メッシュを指定の画像とワールド行列で描くように頼む。
	/// </summary>
	virtual void Submit(MeshHandle mesh, TextureHandle texture, const Matrix4x4& world) = 0;
};

} // namespace KujataEngine
