#include "Framework/Scene.h"

namespace KujataEngine {

Scene::~Scene() {
	for (GameObject* gameObject : gameObjects_) {
		delete gameObject;
	}
}

GameObject* Scene::CreateGameObject() {
	GameObject* gameObject = new GameObject();
	gameObjects_.push_back(gameObject);
	return gameObject;
}

void Scene::Update() {
	for (GameObject* gameObject : gameObjects_) {
		if (gameObject->IsActive()) {
			gameObject->Update();
		}
	}
}

void Scene::UpdateWorldTransforms() {
	for (GameObject* gameObject : gameObjects_) {
		gameObject->GetTransform().UpdateWorldMatrix();
	}
}

void Scene::SubmitRender(IRenderQueue* renderQueue) {
	for (GameObject* gameObject : gameObjects_) {
		if (gameObject->IsActive()) {
			gameObject->SubmitRender(renderQueue);
		}
	}
}

} // namespace KujataEngine
