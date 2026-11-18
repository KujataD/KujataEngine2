#pragma once

#include "Core/Math/Vector2.h"
#include "Core/Math/Vector3.h"
#include "Core/Math/Vector4.h"
#include "Framework/Component.h"
#include "Framework/Scene.h"
#include "Interface/AssetHandle.h"
#include "Interface/IInputActions.h"
#include "Interface/ITime.h"
#include "Interface/IWindow.h"
#include "Scene/Camera.h"

#include <numbers>

/// <summary>
/// 移動し、マウスの方を向いて弾を撃つ自機。
/// </summary>
class PlayerController : public KujataEngine::Component {
public:
	PlayerController(KujataEngine::Scene* scene, KujataEngine::IInputActions* inputActions, KujataEngine::ITime* time, KujataEngine::IWindow* window, KujataEngine::Camera* camera, KujataEngine::MeshHandle bulletMesh, KujataEngine::TextureHandle bulletTexture);

	/// <summary>
	/// 操作に使うアクションを登録する。
	/// </summary>
	static void RegisterActions(KujataEngine::IInputActions* inputActions);

	void Update() override;

	// 1秒に進む距離。
	float moveSpeed = 3.0f;

	// 弾が1秒に進む距離。
	float bulletSpeed = 8.0f;

	// 弾の大きさで、当たり判定にも掛かる。
	float bulletScale = 0.2f;

	// 弾の色。
	KujataEngine::Vector4 bulletTint = {1.0f, 0.35f, 0.1f, 1.0f};

private:
	// アクションの名前。
	static constexpr const char* kMoveAction = "Move";
	static constexpr const char* kFireAction = "Fire";

	// マウスがこれより近いと向きが決まらない。
	static constexpr float kMinAimDistance = 0.01f;

	// モデルの前の向きのずれで、単位はラジアン。
	static constexpr float kModelYawOffset = std::numbers::pi_v<float> * 0.5f;

	/// <summary>
	/// 見た目を指定の向きへ向ける。
	/// </summary>
	void FaceTowards(const KujataEngine::Vector3& direction);

	/// <summary>
	/// 自機の位置から指定の向きへ弾を撃つ。
	/// </summary>
	void SpawnBullet(const KujataEngine::Vector3& direction);

	/// <summary>
	/// 画面の点が指す、高さplaneYの水平面上の点を求める。
	/// </summary>
	bool PickHorizontalPlane(const KujataEngine::Vector2& screenPosition, float planeY, KujataEngine::Vector3* hitPosition);

	KujataEngine::Scene* scene_;
	KujataEngine::IInputActions* inputActions_;
	KujataEngine::ITime* time_;
	KujataEngine::IWindow* window_;
	KujataEngine::Camera* camera_;
	KujataEngine::MeshHandle bulletMesh_;
	KujataEngine::TextureHandle bulletTexture_;
};
