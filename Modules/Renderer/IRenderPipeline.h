#pragma once

#include "RenderContext.h"

namespace KujataEngine {

/// <summary>
/// どの工程をどの順番で描くかを決める、差し替えられる描画の手順。
/// </summary>
class IRenderPipeline {
public:
	virtual ~IRenderPipeline() = default;

	/// <summary>
	/// 1フレーム分の描画の命令を書く。
	/// </summary>
	virtual void Render(const RenderContext& context) = 0;
};

} // namespace KujataEngine
