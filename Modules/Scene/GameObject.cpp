#include "GameObject.h"

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
	for (Component* component : components_) {
		component->Update();
	}
}

void GameObject::SubmitRender(IRenderQueue* renderQueue) {
	for (Component* component : components_) {
		component->SubmitRender(renderQueue);
	}
}

Transform& GameObject::GetTransform() {
	return transform_;
}

} // namespace KujataEngine
