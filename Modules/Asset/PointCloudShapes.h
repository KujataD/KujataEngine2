#pragma once

#include "Interface/AssetData.h"

#include <cstdint>

namespace KujataEngine {

/// <summary>
/// 中心が原点の球の表面に、面積あたりの数が同じになるように点を置いた点群を作る。
/// </summary>
PointCloudData CreateSpherePointCloud(uint32_t pointCount, float radius, const Vector3& color);

/// <summary>
/// 中心が原点でY軸を軸にした円環の表面に、面積あたりの数が同じになるように点を置いた点群を作る。
/// </summary>
PointCloudData CreateTorusPointCloud(uint32_t pointCount, float ringRadius, float tubeRadius, const Vector3& color);

/// <summary>
/// メッシュの表面に、面積あたりの数が同じになるように点を置き、色を画像から読んだ点群を作る。
/// </summary>
PointCloudData CreateMeshSurfacePointCloud(const MeshData& mesh, const ImageData& image, uint32_t pointCount);

} // namespace KujataEngine
