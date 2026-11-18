#pragma once

#include "Interface/AssetData.h"
#include "Interface/ParticleData.h"

#include <cstdint>

namespace KujataEngine {

/// <summary>
/// 中心が原点の球の表面に、面積あたりの数が同じになるように集まる先を置いた形を作る。
/// </summary>
ParticleShape CreateSphereParticleShape(uint32_t pointCount, float radius, const Vector3& color);

/// <summary>
/// 中心が原点でY軸を軸にした円環の表面に、面積あたりの数が同じになるように集まる先を置いた形を作る。
/// </summary>
ParticleShape CreateTorusParticleShape(uint32_t pointCount, float ringRadius, float tubeRadius, const Vector3& color);

/// <summary>
/// 中心が原点で辺が軸に沿った箱の表面に、面積あたりの数が同じになるように集まる先を置いた形を作る。
/// </summary>
ParticleShape CreateBoxParticleShape(uint32_t pointCount, const Vector3& size, const Vector3& color);

/// <summary>
/// メッシュの表面に、面積あたりの数が同じになるように集まる先を置き、色を画像から読んだ形を作る。
/// </summary>
ParticleShape CreateMeshSurfaceParticleShape(const MeshData& mesh, const ImageData& image, uint32_t pointCount);

} // namespace KujataEngine
