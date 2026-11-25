#pragma once

#include <cstdint>

/// <summary>
/// 衝突の絞り込みに使うLayerの番号。
/// </summary>
namespace GameLayer {

// 0は既定の番号なので使わない。
constexpr uint32_t kPlayer = 1;
constexpr uint32_t kPlayerBullet = 2;
constexpr uint32_t kEnemy = 3;
constexpr uint32_t kEnemyBullet = 4;

/// <summary>
/// Layerの番号をビットマスクにする。
/// </summary>
constexpr uint32_t ToMask(uint32_t layer) {
	return 1u << layer;
}

} // namespace GameLayer
