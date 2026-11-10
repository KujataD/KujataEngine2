#pragma once

#include "IMeshFileLoader.h"

namespace KujataEngine {

/// <summary>
/// Wavefrontの.objと、それが指す.mtlの画像の名前を読むクラス。
/// </summary>
class ObjMeshLoader : public IMeshFileLoader {
public:
	bool Load(const std::filesystem::path& filePath, LoadedModelFile* result) override;
};

} // namespace KujataEngine
