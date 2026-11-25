#pragma once

#include "Core/Math/Vector3.h"
#include "Core/Math/Vector4.h"
#include "SceneGraph/Component.h"
#include "SceneGraph/Scene.h"
#include "Interface/AssetHandle.h"
#include "Interface/ITime.h"

#include <cstdint>

/// <summary>
/// 決まった間隔で弾を撃つ部品。
/// </summary>
class Weapon : public KujataEngine::Component {
public:
	Weapon(KujataEngine::Scene* scene, KujataEngine::ITime* time, KujataEngine::MeshHandle bulletMesh, KujataEngine::TextureHandle bulletTexture);

	void Update() override;

	/// <summary>
	/// 間隔が空いていれば指定の向きへ弾を撃ち、trueを返す。
	/// </summary>
	bool TryFire(const KujataEngine::Vector3& direction);

	// 弾と弾の間隔で、単位は秒。
	float interval = 0.1f;

	// 弾が1秒に進む距離。
	float bulletSpeed = 8.0f;

	// 弾が1発で減らすHP。
	int bulletDamage = 4;

	// 弾の大きさで、当たり判定にも掛かる。
	float bulletScale = 0.2f;

	// 弾の色。
	KujataEngine::Vector4 bulletTint = {1.0f, 0.35f, 0.1f, 1.0f};

	// 持ち主の中心から、弾を出す位置までの距離。
	float muzzleDistance = 0.4f;

	// 弾のLayerと、弾が当たる相手のLayerのマスク。
	uint32_t bulletLayer = 0;
	uint32_t targetLayerMask = 0;

private:
	KujataEngine::Scene* scene_;
	KujataEngine::ITime* time_;
	KujataEngine::MeshHandle bulletMesh_;
	KujataEngine::TextureHandle bulletTexture_;

	// 次に撃てるまでの秒数。
	float cooldownSeconds_ = 0.0f;
};
