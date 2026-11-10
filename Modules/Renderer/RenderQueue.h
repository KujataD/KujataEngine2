#pragma once

#include "Interface/IRenderQueue.h"

#include <vector>

namespace KujataEngine {

/// <summary>
/// 1回分の描画の依頼。
/// </summary>
struct DrawRequest {
	MeshHandle mesh;
	TextureHandle texture;
	Matrix4x4 world;
};

/// <summary>
/// コンポーネントから届いた描画の依頼を、描くまで預かるクラス。
/// </summary>
class RenderQueue : public IRenderQueue {
public:
	void SetCamera(const CameraData& camera) override;
	void Submit(MeshHandle mesh, TextureHandle texture, const Matrix4x4& world) override;

	bool HasCamera() const;
	const CameraData& GetCamera() const;
	const std::vector<DrawRequest>& GetDrawRequests() const;

	/// <summary>
	/// 次のフレームのために預かった依頼を空にする。
	/// </summary>
	void Clear();

private:
	CameraData camera_ = {};
	bool hasCamera_ = false;
	std::vector<DrawRequest> drawRequests_;
};

} // namespace KujataEngine
