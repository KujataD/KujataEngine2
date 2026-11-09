#include "Framework/EntityAllocator.h"

#include "Core/Assert.h"

namespace KujataEngine {

Entity EntityAllocator::Create() {
	if (!freeIndices_.empty()) {
		const uint32_t index = freeIndices_.back();
		freeIndices_.pop_back();
		return { index, generations_[index] };
	}

	const uint32_t index = static_cast<uint32_t>(generations_.size());
	generations_.push_back(0);
	return { index, 0 };
}

void EntityAllocator::Destroy(Entity entity) {
	KUJATA_ASSERT(IsAlive(entity));

	generations_[entity.index]++;
	freeIndices_.push_back(entity.index);
}

bool EntityAllocator::IsAlive(Entity entity) const {
	return entity.index < generations_.size() && generations_[entity.index] == entity.generation;
}

} // namespace KujataEngine
