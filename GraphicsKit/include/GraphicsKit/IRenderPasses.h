#pragma once

#include "GraphicsKit/IRenderPass.h"

namespace KujataEngine {

// ポインタで渡すだけなので、クラスの名前だけを伝える。
class ShaderCompiler;
class ShaderResourceHeap;

// ServiceRegistryに登録するときの名前。
const char* const kRenderPassesServiceName = "RenderPasses";

/// <summary>
/// 工程を描く順番の大きなくくりで、上から順に描く。
/// </summary>
enum class RenderStage {
	// 深度を書く不透明な物。
	Opaque,

	// 深度を比べるだけで書かずに、後ろの色と混ぜる物。
	Transparent,

	// 深度を書かずに明るさを足す物。
	Additive,

	// 確かめるための線など、最後に重ねる物。
	Debug,
};

/// <summary>
/// 工程を足すための窓口で、工程が共有する部品も貸す。
/// </summary>
class IRenderPasses {
public:
	virtual ~IRenderPasses() = default;

	/// <summary>
	/// 工程を足し、段階の順、同じ段階では番号の小さい順、同じ番号なら足した順に描く。工程の持ち主は呼び出し側のままにする。
	/// </summary>
	virtual void AddPass(IRenderPass* pass, RenderStage stage, int order) = 0;

	/// <summary>
	/// シェーダーをコンパイルする部品を返す。
	/// </summary>
	virtual ShaderCompiler* GetShaderCompiler() = 0;

	/// <summary>
	/// シェーダーから見える名札の棚を返し、どの工程も同じ棚を使う。
	/// </summary>
	virtual ShaderResourceHeap* GetShaderResourceHeap() = 0;
};

} // namespace KujataEngine
