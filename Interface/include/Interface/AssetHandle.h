#pragma once

#include <cstdint>

namespace KujataEngine {

/// <summary>
/// 読み込んだメッシュを指す番号で、0番は代わりの立方体を表す。
/// </summary>
struct MeshHandle {
	uint32_t id = 0;
};

/// <summary>
/// 読み込んだ画像を指す番号で、0番は代わりのピンクの画像を表す。
/// </summary>
struct TextureHandle {
	uint32_t id = 0;
};

/// <summary>
/// モデルを描くのに必要なメッシュと画像の組。
/// </summary>
struct ModelHandles {
	MeshHandle mesh;
	TextureHandle texture;
};

} // namespace KujataEngine
