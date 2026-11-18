#pragma once

#include "Core/Math/Vector2.h"
#include "Core/Math/Vector3.h"
#include "Framework/Component.h"
#include "Interface/IInputActions.h"
#include "Interface/ITime.h"
#include "Interface/IWindow.h"
#include "Scene/Camera.h"
#include "Weapon.h"

#include <numbers>

/// <summary>
/// 移動し、マウスの方を向いて撃つ自機。
/// </summary>
class PlayerController : public KujataEngine::Component {
public:
	PlayerController(KujataEngine::IInputActions* inputActions, KujataEngine::ITime* time, KujataEngine::IWindow* window, KujataEngine::Camera* camera, Weapon* weapon);

	/// <summary>
	/// 操作に使うアクションを登録する。
	/// </summary>
	static void RegisterActions(KujataEngine::IInputActions* inputActions);

	void Update() override;

	// 1秒に進む距離。
	float moveSpeed = 3.0f;

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
	/// 画面の点が指す、高さplaneYの水平面上の点を求める。
	/// </summary>
	bool PickHorizontalPlane(const KujataEngine::Vector2& screenPosition, float planeY, KujataEngine::Vector3* hitPosition);

	KujataEngine::IInputActions* inputActions_;
	KujataEngine::ITime* time_;
	KujataEngine::IWindow* window_;
	KujataEngine::Camera* camera_;
	Weapon* weapon_;
};
