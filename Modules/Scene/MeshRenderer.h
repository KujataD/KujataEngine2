#pragma once

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

private:
	MeshHandle mesh_;
	TextureHandle texture_;
};

} // namespace KujataEngine
