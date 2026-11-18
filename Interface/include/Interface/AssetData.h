#pragma once

#include "Core/Math/Vector2.h"
#include "Core/Math/Vector3.h"

#include <cstdint>
#include <vector>

namespace KujataEngine {

/// <summary>
/// 頂点1個分のデータで、PipelineStateの入力とObject3D.VS.hlslのVertexInputと並びを合わせる。
/// </summary>
struct MeshVertex {
	Vector3 position;
	Vector2 uv;
	Vector3 normal;
};

/// <summary>
/// 形を表すデータで、GPUには依存しない。
/// </summary>
struct MeshData {
	std::vector<MeshVertex> vertices;
	std::vector<uint32_t> indices;
};

/// <summary>
/// 画像の画素の形式で、特定のグラフィックスAPIには依存しない。
/// </summary>
enum class ImageFormat {
	Rgba8Srgb,
};

/// <summary>
/// ミップマップの1段分の画素。
/// </summary>
struct ImageMip {
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t rowPitch = 0;
	std::vector<uint8_t> pixels;
};

/// <summary>
/// ミップマップを全段含む画像のデータ。
/// </summary>
struct ImageData {
	ImageFormat format = ImageFormat::Rgba8Srgb;
	std::vector<ImageMip> mips;
};

} // namespace KujataEngine
