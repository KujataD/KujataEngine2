#pragma once

namespace KujataEngine {

// ポインタで受け取るだけなので、クラスの名前だけを伝える。
class IRenderQueue;

/// <summary>
/// 描きたい物があるコンポーネントが継承する窓口。
/// </summary>
class IRenderable {
public:
	virtual ~IRenderable() = default;

	/// <summary>
	/// 描きたい物を毎フレーム描画の担当へ頼む。
	/// </summary>
	virtual void SubmitRender(IRenderQueue* renderQueue) = 0;
};

} // namespace KujataEngine
