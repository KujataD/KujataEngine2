#pragma once

#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"
#include "Interface/AssetHandle.h"
#include "Interface/IRenderable.h"
#include "SceneGraph/Component.h"

namespace KujataEngine {

/// <summary>
/// 付いているGameObjectの位置に、ライトを通さず画像を1枚描くコンポーネント。
/// </summary>
class SpriteRenderer : public Component, public IRenderable {
public:
	/// <summary>
	/// quadはMakeSpriteQuadで作って登録した四角形で、起動のときに転送されるので、シーンを組む前に1回だけ作って使い回す。
	/// </summary>
	SpriteRenderer(MeshHandle quad, TextureHandle texture);

	void SubmitRender(IRenderQueue* renderQueue) override;

	// 画像の色に掛ける色で、アルファを下げると後ろが透ける。
	Vector4 color = {1.0f, 1.0f, 1.0f, 1.0f};

	// 描く順で、大きいほど手前になる。
	int sortingOrder = 0;

	// 画像のどこを使うかで、移動と拡大は画像全体を1とした割合。
	Vector2 uvOffset = {0.0f, 0.0f};
	Vector2 uvScale = {1.0f, 1.0f};

	// UVを回す角度で、単位はラジアン。
	float uvRotation = 0.0f;

private:
	MeshHandle quad_;
	TextureHandle texture_;
};

} // namespace KujataEngine
