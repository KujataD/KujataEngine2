#include "Framework/Scene.h"

#include <algorithm>

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
	// 途中で作った物で一覧が伸びても壊れないように、始めた時点の数だけを番号で回す。
	const size_t objectCount = gameObjects_.size();
	for (size_t i = 0; i < objectCount; i++) {
		GameObject* gameObject = gameObjects_[i];
		if (gameObject->IsActive() && !gameObject->IsDestroyed()) {
			gameObject->Update();
		}
	}
}

void Scene::AddDestroyListener(std::function<void(GameObject*)> listener) {
	destroyListeners_.push_back(listener);
}

void Scene::FlushDestroyed() {
	// 先に集めておくので、この処理の中で新しく印を付けた物は次のフレームで消える。
	std::vector<GameObject*> destroyedObjects;
	for (GameObject* gameObject : gameObjects_) {
		if (gameObject->IsDestroyed()) {
			destroyedObjects.push_back(gameObject);
		}
	}

	// 触れていた相手への通知とOnDestroyは、消す物がまだ全部残っているうちに行う。
	for (GameObject* gameObject : destroyedObjects) {
		for (const std::function<void(GameObject*)>& listener : destroyListeners_) {
			listener(gameObject);
		}
		gameObject->CallOnDestroy();
	}

	for (GameObject* gameObject : destroyedObjects) {
		gameObjects_.erase(std::find(gameObjects_.begin(), gameObjects_.end(), gameObject));
		delete gameObject;
	}
}

void Scene::UpdateWorldTransforms() {
	for (GameObject* gameObject : gameObjects_) {
		gameObject->GetTransform().UpdateWorldMatrix();
	}
}

} // namespace KujataEngine
