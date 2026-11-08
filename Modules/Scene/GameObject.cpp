#include "GameObject.h"

namespace KujataEngine {

GameObject::~GameObject() {
	for (Component* component : components_) {
		delete component;
	}
}

void GameObject::AddComponent(Component* component) {
	component->SetOwner(this);
	components_.push_back(component);
}

void GameObject::Update() {
	for (Component* component : components_) {
		component->Update();
	}
}

} // namespace KujataEngine
