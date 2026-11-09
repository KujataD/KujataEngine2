#pragma once

#include <Core/Assert.h>
#include <Framework/Entity.h>

#include <cstddef>
#include <cstdint>
#include <vector>

namespace KujataEngine {

/// <summary>
/// 中身の型を知らなくてもEntityの行を消せるようにする窓口。
/// </summary>
class IComponentStorage {
public:
	virtual ~IComponentStorage() = default;

	/// <summary>
	/// このEntityの行があれば消す。
	/// </summary>
	virtual void RemoveIfExists(Entity entity) = 0;
};

/// <summary>
/// 1種類のコンポーネントを、隙間のない配列になれべ手持つコンテナ。
/// </summary>
/// <typeparam name="T">コンポーネント</typeparam>
template<typename T> class ComponentStorage : public IComponentStorage {
public:
	ComponentStorage() = default;
	~ComponentStorage() = default;

	/// <summary>
	/// Entityにコンポーネントを付ける。
	/// </summary>
	void Add(Entity entity, const T& component) {
		KUJATA_ASSERT(!Has(entity));

		// indexに追加されるたびサイズを大きくする。
		if (entity.index >= sparse_.size()) {
			sparse_.resize(entity.index + 1, kEmpty);
		}

		sparse_[entity.index] = static_cast<uint32_t>(entities_.size());
		entities_.push_back(entity);
		components_.push_back(component);
	}

	/// <summary>
	/// このEntityの行があればtrueを返す。
	/// </summary>
	bool Has(Entity entity) const {
		if (entity.index >= sparse_.size()) {
			return false;
		}

		const uint32_t denseIndex = sparse_[entity.index];
		return denseIndex != kEmpty && entities_[denseIndex] == entity;
	}

	/// <summary>
	/// このEntityのコンポーネントを返す。
	/// </summary>
	T& Get(Entity entity) {
		KUJATA_ASSERT(Has(entity));
		return components_[sparse_[entity.index]];
	}

	/// <summary>
	/// 末尾の行を消した場所へ移して、隙間を作らずに消す。
	/// </summary>
	void Remove(Entity entity) {
		KUJATA_ASSERT(Has(entity));

		const uint32_t removedIndex = sparse_[entity.index];
		const Entity movedEntity = entities_.back();

		entities_[removedIndex] = movedEntity;
		components_[removedIndex] = components_.back();
		sparse_[movedEntity.index] = removedIndex;

		// 消すのが末尾でも正しく空になるように、移動の後で空にする。
		sparse_[entity.index] = kEmpty;

		entities_.pop_back();
		components_.pop_back();
	}

	void RemoveIfExists(Entity entity) override {
		if (Has(entity)) {
			Remove(entity);
		}
	}

	const std::vector<Entity>& Entities() const { return entities_; }
	size_t Size() const { return entities_.size(); }
	Entity EntityAt(size_t denseIndex) const { return entities_[denseIndex]; }
	T& ComponentAt(size_t denseIndex) { return components_[denseIndex]; }

private:
	// 席番号に行がないことを表す値。
	static constexpr uint32_t kEmpty = UINT32_MAX;

	// 席番号から、配列の何行目かを引く表。
	std::vector<uint32_t> sparse_;
	// 行ごとの持ち主。
	std::vector<Entity> entities_;
	// 行ごとのデータ。
	std::vector<T> components_;
};

} // namespace KujataEngine