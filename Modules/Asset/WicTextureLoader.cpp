#include "WicTextureLoader.h"

#include <d3d12.h>

#include "../../externals/DirectXTex/DirectXTex.h"

#include <utility>

namespace KujataEngine {

// Rendererへ渡す画素の形式で、ImageFormatのRgba8Srgbに対応する。
static const DXGI_FORMAT kImageFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

/// <summary>
/// DirectXTexの画像の全段を、Rendererへ渡すデータにコピーする。
/// </summary>
static void CopyMips(const DirectX::ScratchImage& source, ImageData* result) {
	result->format = ImageFormat::Rgba8Srgb;
	result->mips.clear();

	for (size_t i = 0; i < source.GetImageCount(); i++) {
		const DirectX::Image& image = source.GetImages()[i];

		ImageMip mip;
		mip.width = static_cast<uint32_t>(image.width);
		mip.height = static_cast<uint32_t>(image.height);
		mip.rowPitch = static_cast<uint32_t>(image.rowPitch);
		mip.pixels.assign(image.pixels, image.pixels + image.slicePitch);
		result->mips.push_back(std::move(mip));
	}
}

bool WicTextureLoader::Load(const std::filesystem::path& filePath, ImageData* result) {
	DirectX::ScratchImage image;
	if (FAILED(DirectX::LoadFromWICFile(filePath.c_str(), DirectX::WIC_FLAGS_FORCE_RGB | DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image))) {
		return false;
	}

	// 16ビットの画像なども、Rendererが読める形式にそろえる。
	if (image.GetMetadata().format != kImageFormat) {
		DirectX::ScratchImage converted;
		if (FAILED(DirectX::Convert(image.GetImages(), image.GetImageCount(), image.GetMetadata(), kImageFormat, DirectX::TEX_FILTER_SRGB_IN, DirectX::TEX_THRESHOLD_DEFAULT, converted))) {
			return false;
		}
		image = std::move(converted);
	}

	// 1x1の画像はこれ以上小さくできないので、そのまま使う。
	const bool canGenerateMipMaps = image.GetMetadata().width > 1 || image.GetMetadata().height > 1;
	if (!canGenerateMipMaps) {
		CopyMips(image, result);
		return true;
	}

	DirectX::ScratchImage mipImages;
	if (FAILED(DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages))) {
		return false;
	}

	CopyMips(mipImages, result);
	return true;
}

} // namespace KujataEngine
