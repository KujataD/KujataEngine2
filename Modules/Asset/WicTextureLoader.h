#pragma once

#include "ITextureFileLoader.h"

namespace KujataEngine {

/// <summary>
/// PNGやJPEGをWindowsの機能で読み、ミップマップまで作るクラス。
/// </summary>
class WicTextureLoader : public ITextureFileLoader {
public:
	bool Load(const std::filesystem::path& filePath, ImageData* result) override;
};

} // namespace KujataEngine
