#pragma once

#include "Framework/GameObject.h"

#include <functional>
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
	/// 持っているGameObjectを、OnDestroyを呼ばずにすべてdeleteする。
	/// </summary>
	~Scene();

	/// <summary>
	/// GameObjectを作って返し、消す責任はSceneが持つ。
	/// </summary>
	GameObject* CreateGameObject();

	/// <summary>
	/// 新しいComponentのStartを全部済ませてから作った順に更新し、途中で作った物は次のフレームから動く。
	/// </summary>
	void Update();

	/// <summary>
	/// 消える直前の物を受け取る処理を登録し、触れていた相手への後始末などに使う。
	/// </summary>
	void AddDestroyListener(std::function<void(GameObject*)> listener);

	/// <summary>
	/// 消す印の付いた物を、聞き手への通知とOnDestroyの後に消す。
	/// </summary>
	void FlushDestroyed();

	/// <summary>
	/// 全GameObjectのワールド行列を今の位置に合わせる。
	/// </summary>
	void UpdateWorldTransforms();

	const std::vector<GameObject*>& GetGameObjects() const { return gameObjects_; }

private:
	std::vector<GameObject*> gameObjects_;
	std::vector<std::function<void(GameObject*)>> destroyListeners_;
};

} // namespace KujataEngine
