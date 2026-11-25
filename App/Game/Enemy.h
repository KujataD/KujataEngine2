#pragma once

#include "Core/Math/Vector3.h"
#include "Interface/ITime.h"
#include "SceneGraph/Component.h"
#include "SceneGraph/GameObject.h"
#include "Weapon.h"

/// <summary>
/// 狙う相手へ揺れながら近づいて撃ち続け、HPがなくなると消える敵。
/// </summary>
class Enemy : public KujataEngine::Component {
public:
	/// <summary>
	/// noiseSeedを1より大きく離すと、敵ごとに別の揺れ方になる。
	/// </summary>
	Enemy(KujataEngine::ITime* time, Weapon* weapon, KujataEngine::GameObject* target, float noiseSeed);

	void Update() override;

	// 相手へ向かって1秒に進む距離。
	float chaseSpeed = 1.5f;

	// 相手にこれより近づいたら、向かうのをやめる距離。
	float keepDistance = 3.0f;

	// ノイズの揺れで1秒に進む距離の目安。
	float wanderSpeed = 1.0f;

	// 揺れの速さで、大きいほどせわしなく向きを変える。
	float wanderFrequency = 0.5f;

private:
	// 相手がこれより近いと向きが決まらない。
	static constexpr float kMinAimDistance = 0.01f;

	// xとzを別の揺れにするために、ノイズの行を離す幅。
	static constexpr float kAxisRowOffset = 100.0f;

	/// <summary>
	/// 相手へ向かう速度とノイズの速度を足して、1回だけ動かす。
	/// </summary>
	void Move(const KujataEngine::Vector3& aimDirection, float targetDistance);

	KujataEngine::ITime* time_;
	Weapon* weapon_;
	KujataEngine::GameObject* target_;
	float noiseSeed_;

	// 動き始めてからの秒数で、ノイズの時間に使う。
	float elapsedSeconds_ = 0.0f;
};
