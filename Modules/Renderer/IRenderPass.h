#pragma once

#include "RenderContext.h"

namespace KujataEngine {

/// <summary>
/// 描画の手順の中の1工程。
/// </summary>
class IRenderPass {
public:
	virtual ~IRenderPass() = default;

	/// <summary>
	/// この工程の描画の命令を書く。
	/// </summary>
	virtual void Execute(const RenderContext& context) = 0;
};

} // namespace KujataEngine
