#include "PlayerController.h"

#include "Core/Math/Matrix4x4.h"
#include "Framework/GameObject.h"

#include <cmath>

using namespace KujataEngine;

PlayerController::PlayerController(IInputActions* inputActions, ITime* time, IWindow* window, Camera* camera, Weapon* weapon) {
	inputActions_ = inputActions;
	time_ = time;
	window_ = window;
	camera_ = camera;
	weapon_ = weapon;
}

void PlayerController::RegisterActions(IInputActions* inputActions) {
	inputActions->AddAxis2D(kMoveAction);
	inputActions->BindKeys2D(kMoveAction, Key::W, Key::S, Key::A, Key::D);
	inputActions->BindKeys2D(kMoveAction, Key::Up, Key::Down, Key::Left, Key::Right);
	inputActions->BindPadStick(kMoveAction, PadStick::Left);

	inputActions->AddButton(kFireAction);
	inputActions->BindKey(kFireAction, Key::Space);
	inputActions->BindPadButton(kFireAction, PadButton::A);
	inputActions->BindMouseButton(kFireAction, MouseButton::Left);
}

void PlayerController::Update() {
	WorldTransform& transform = GetOwner()->GetTransform();

	// 2軸の上を奥にして、床の上を動く。
	const Vector2 move = inputActions_->GetAxis2D(kMoveAction);
	const float distance = moveSpeed * time_->GetDeltaTime();
	transform.translation_.x += move.x * distance;
	transform.translation_.z += move.y * distance;

	// 最小化中は狙えないので何もしない。
	Vector3 aimPoint;
	if (!PickHorizontalPlane(inputActions_->GetMousePosition(), transform.translation_.y, &aimPoint)) {
		return;
	}

	// 自機からマウスへの水平な向きで、見た目と弾の両方に使う。
	Vector3 toAim = aimPoint - transform.translation_;
	toAim.y = 0.0f;
	if (Length(toAim) < kMinAimDistance) {
		return;
	}
	const Vector3 aimDirection = Normalize(toAim);
	FaceTowards(aimDirection);

	// 押している間は、Weaponの間隔ごとに撃ち続ける。
	if (inputActions_->IsHeld(kFireAction)) {
		weapon_->TryFire(aimDirection);
	}
}

void PlayerController::FaceTowards(const Vector3& direction) {
	// Zの正の向きを0とした角度に、モデルのずれを足す。
	GetOwner()->GetTransform().rotation_.y = std::atan2(direction.x, direction.z) + kModelYawOffset;
}


bool PlayerController::PickHorizontalPlane(const Vector2& screenPosition, float planeY, Vector3* hitPosition) {
	// マウスの位置はウィンドウのピクセルなので、ウィンドウの大きさを使う。
	const float width = static_cast<float>(window_->GetClientWidth());
	const float height = static_cast<float>(window_->GetClientHeight());
	if (width <= 0.0f || height <= 0.0f) {
		return false;
	}

	// 今映っている絵の、前のフレームのカメラの行列。
	const Matrix4x4 view = Inverse(camera_->GetOwner()->GetTransform().matWorld_);
	const Matrix4x4 projection = MakePerspectiveFovMatrix(camera_->fovYRadians, width / height, camera_->nearZ, camera_->farZ);
	const Matrix4x4 viewport = MakeViewportMatrix(0.0f, 0.0f, width, height, 0.0f, 1.0f);

	// 描画と同じ行列の逆で、画面の点を世界へ戻す。
	const Matrix4x4 screenToWorld = Inverse(view * projection * viewport);

	// 深さ0が写す範囲の手前の面に、1が奥の面に当たる。
	const Vector3 nearPoint = Transform({screenPosition.x, screenPosition.y, 0.0f}, screenToWorld);
	const Vector3 farPoint = Transform({screenPosition.x, screenPosition.y, 1.0f}, screenToWorld);
	const Vector3 ray = farPoint - nearPoint;

	// 線が面と平行だと交わらない。
	if (std::abs(ray.y) < 1e-6f) {
		return false;
	}

	// 負なら面がカメラの後ろにある。
	const float t = (planeY - nearPoint.y) / ray.y;
	if (t < 0.0f) {
		return false;
	}

	*hitPosition = nearPoint + ray * t;
	return true;
}
