#pragma once

#include <cstdint>

namespace KujataEngine {

/// <summary>
/// ワールドオブジェクトを表す番号。
/// </summary>
struct Entity {
	// 何番の席かを表す。
	uint32_t index = 0;
	// その席が何代目の持ち主かを表す。
	uint32_t generation = 0;

	bool operator==(const Entity& other) const = default;
};

} // namespace KujataEngine