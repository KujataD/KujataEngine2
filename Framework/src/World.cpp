#include "Framework/World.h"

namespace KujataEngine {

size_t World::NextTypeId() {
	static size_t nextTypeId = 0;
	return nextTypeId++;
}

Entity World::CreateEntity() {
	return allocator_.Create();
}

void World::DestroyEntity(Entity entity) {
	KUJATA_ASSERT(IsAlive(entity));

	for (std::unique_ptr<IComponentStorage>& storage : storages_) {
		if (storage != nullptr) {
			storage->RemoveIfExists(entity);
		}
	}

	allocator_.Destroy(entity);
}

bool World::IsAlive(Entity entity) const {
	return allocator_.IsAlive(entity);
}

} // namespace KujataEngine
