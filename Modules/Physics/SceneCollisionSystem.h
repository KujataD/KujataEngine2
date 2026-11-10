#pragma once

#include <string>
#include <map>

namespace KujataEngine {

class Scene;
class ColliderComponent;

/// <summary>
/// Scene内Colliderの衝突検出(Sweep and Prune)と、Enter/Stay/Exitイベントの通知。
/// 非トリガーの交差にはScenePhysicsで剛体応答を適用する。
/// 前フレームの接触ペアを覚えておき、今フレームとの差分でEnter/Exitを決める。
/// </summary>
class SceneCollisionSystem {
public:
	/// <summary>
	/// 全Colliderの交差を調べ、剛体応答とイベント通知を行う。ワールド行列を更新してから呼ぶこと。
	/// </summary>
	void Update(Scene& scene);

	/// <summary>
	/// 接触ペアの記憶を、Exitイベントを送らずに捨てる。
	/// </summary>
	void Clear() { pairStates_.clear(); }

private:
	struct PairState {
		ColliderComponent* colliderA = nullptr;
		ColliderComponent* colliderB = nullptr;
		bool isTrigger = false;
	};

	// 通知の順番が毎回同じになるように、順序付きのmapで持つ。
	std::map<std::string, PairState> pairStates_;
};

} // namespace KujataEngine
