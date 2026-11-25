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
	Vector4 tint;
};

/// <summary>
/// スプライトの依頼を預かるための入れ物。
/// </summary>
struct SpriteRequest {
	MeshHandle quad;
	TextureHandle texture;
	Matrix4x4 world;
	Vector4 color;
	Matrix4x4 uvTransform;
	int sortingOrder;
};

/// <summary>
/// コンポーネントから届いた描画の依頼を、描くまで預かるクラス。
/// </summary>
class RenderQueue : public IRenderQueue {
public:
	void SetCamera(const CameraData& camera) override;
	void SetDirectionalLight(const DirectionalLightData& light) override;
	void Submit(MeshHandle mesh, TextureHandle texture, const Matrix4x4& world, const Vector4& tint) override;
	void SubmitSprite(MeshHandle quad, TextureHandle texture, const Matrix4x4& world, const Vector4& color, const Matrix4x4& uvTransform, int sortingOrder) override;

	bool HasCamera() const;
	const CameraData& GetCamera() const;
	bool HasDirectionalLight() const;
	const DirectionalLightData& GetDirectionalLight() const;
	const std::vector<DrawRequest>& GetDrawRequests() const;
	const std::vector<SpriteRequest>& GetSpriteRequests() const;

	/// <summary>
	/// 次のフレームのために預かった依頼を空にする。
	/// </summary>
	void Clear();

private:
	CameraData camera_ = {};
	bool hasCamera_ = false;
	DirectionalLightData directionalLight_ = {};
	bool hasDirectionalLight_ = false;
	std::vector<DrawRequest> drawRequests_;
	std::vector<SpriteRequest> spriteRequests_;
};

} // namespace KujataEngine
