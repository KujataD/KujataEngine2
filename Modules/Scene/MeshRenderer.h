#pragma once

#include "Core/Math/Vector4.h"
#include "Framework/Component.h"
#include "Interface/AssetHandle.h"
#include "Interface/IRenderable.h"

namespace KujataEngine {

/// <summary>
/// 付いているGameObjectの位置にメッシュを描くコンポーネント。
/// </summary>
class MeshRenderer : public Component, public IRenderable {
public:
	MeshRenderer(MeshHandle mesh, TextureHandle texture);

	void SubmitRender(IRenderQueue* renderQueue) override;

	// 画像の色に掛ける色。
	Vector4 tint = {1.0f, 1.0f, 1.0f, 1.0f};

private:
	MeshHandle mesh_;
	TextureHandle texture_;
};

} // namespace KujataEngine
