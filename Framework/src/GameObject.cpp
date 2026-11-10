#include "Framework/GameObject.h"

#include "Core/Assert.h"

namespace KujataEngine {

GameObject::~GameObject() {
	for (Component* component : components_) {
		delete component;
	}
}

void GameObject::AddComponent(Component* component) {
	KUJATA_ASSERT(component != nullptr);

	component->SetOwner(this);
	components_.push_back(component);
}

void GameObject::Update() {
	// 途中で足されたComponentで一覧が伸びても壊れないように、始めた時点の数だけを番号で回す。
	const size_t componentCount = components_.size();
	for (size_t i = 0; i < componentCount; i++) {
		Component* component = components_[i];
		if (!component->IsEnabled()) {
			continue;
		}

		component->StartIfNeeded();
		component->Update();
	}
}

void GameObject::CallOnDestroy() {
	const size_t componentCount = components_.size();
	for (size_t i = 0; i < componentCount; i++) {
		components_[i]->OnDestroy();
	}
}

WorldTransform& GameObject::GetTransform() {
	return transform_;
}

const WorldTransform& GameObject::GetTransform() const {
	return transform_;
}

void GameObject::UpdateWorldTransformSelfAndAncestors() {
	transform_.UpdateWorldMatrix();
}

} // namespace KujataEngine
