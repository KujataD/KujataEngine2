#pragma once

#include "Framework/GameObject.h"

#include <vector>

namespace KujataEngine {

// ServiceRegistryに登録するときの名前。
const char* const kSceneServiceName = "Scene";

/// <summary>
/// GameObjectを集めて持ち、作った順に動かす入れ物。
/// </summary>
class Scene {
public:
	/// <summary>
	/// 持っているGameObjectをすべてdeleteする。
	/// </summary>
	~Scene();

	/// <summary>
	/// GameObjectを作って返し、消す責任はSceneが持つ。
	/// </summary>
	GameObject* CreateGameObject();

	void Update();

	/// <summary>
	/// 全GameObjectのワールド行列を今の位置に合わせる。
	/// </summary>
	void UpdateWorldTransforms();

	const std::vector<GameObject*>& GetGameObjects() const { return gameObjects_; }

private:
	std::vector<GameObject*> gameObjects_;
};

} // namespace KujataEngine
