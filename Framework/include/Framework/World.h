#pragma once

#include "Core/Assert.h"
#include "Framework/ComponentStorage.h"
#include "Framework/EntityAllocator.h"

#include <cstddef>
#include <memory>
#include <vector>

namespace KujataEngine {

/// <summary>
/// Entityと、型ごとのコンポーネントの入れ物をまとめて持つ世界。
/// </summary>
class World {
public:
	Entity CreateEntity();

	/// <summary>
	/// 全部の入れ物からこのEntityの行を消して、番号を返却する。
	/// </summary>
	void DestroyEntity(Entity entity);

	bool IsAlive(Entity entity) const;

	template<typename T> void Add(Entity entity, const T& component) {
		KUJATA_ASSERT(IsAlive(entity));
		GetStorage<T>().Add(entity, component);
	}

	template<typename T> bool Has(Entity entity) {
		return GetStorage<T>().Has(entity);
	}

	template<typename T> T& Get(Entity entity) {
		return GetStorage<T>().Get(entity);
	}

	template<typename T> void Remove(Entity entity) {
		GetStorage<T>().Remove(entity);
	}

private:
	static size_t NextTypeId();

	/// <summary>
	/// 型ごとに1回だけ番号を決めて、その番号を返す。
	/// </summary>
	template<typename T> static size_t GetTypeId() {
		static const size_t typeId = NextTypeId();
		return typeId;
	}

	/// <summary>
	/// 型に対応する入れ物を返し、まだなければ作る。
	/// </summary>
	template<typename T> ComponentStorage<T>& GetStorage() {
		const size_t typeId = GetTypeId<T>();

		if (typeId >= storages_.size()) {
			storages_.resize(typeId + 1);
		}
		if (storages_[typeId] == nullptr) {
			storages_[typeId] = std::make_unique<ComponentStorage<T>>();
		}

		return *static_cast<ComponentStorage<T>*>(storages_[typeId].get());
	}

	EntityAllocator allocator_;

	// 型の番号を添字にした、入れ物の一覧。
	std::vector<std::unique_ptr<IComponentStorage>> storages_;
};

} // namespace KujataEngine
