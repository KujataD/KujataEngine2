#pragma once

#include "Framework/Entity.h"

#include <cstdint>
#include <vector>

namespace KujataEngine {

/// <summary>
/// Entityの番号を配り、消された番号を再利用する係。
/// </summary>
class EntityAllocator {
public:
	/// <summary>
	/// 空いている席があれば再利用して、新しいEntityを返す。
	/// </summary>
	Entity Create();

	/// <summary>
	/// 席の世代を進めて、古いEntityを無効にする。
	/// </summary>
	void Destroy(Entity entity);

	/// <summary>
	/// 消されていないEntityならtrueを返す。
	/// </summary>
	bool IsAlive(Entity entity) const;

private:
	// 席ごとの今の世代。
	std::vector<uint32_t> generations_;
	// 空いている席の番号。
	std::vector<uint32_t> freeIndices_;
};

} // namespace KujataEngine