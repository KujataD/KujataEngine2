#pragma once

#include "GameObject.h"

#include <vector>

namespace KujataEngine {

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
	void SubmitRender(IRenderQueue* renderQueue);

private:
	std::vector<GameObject*> gameObjects_;
};

} // namespace KujataEngine
