#include "DebugCamera.h"

#include "SceneGraph/GameObject.h"
#include "Interface/IRenderQueue.h"

#include <cmath>
#include <cstdio>

namespace KujataEngine {

// デバッグカメラの操作に使うアクションの名前。
static const char* const kMoveAction = "DebugCameraMove";
static const char* const kUpAction = "DebugCameraUp";
static const char* const kDownAction = "DebugCameraDown";
static const char* const kLookAction = "DebugCameraLook";
static const char* const kLookEnabledAction = "DebugCameraLookEnabled";

// 1秒に進む距離。
static const float kMoveSpeed = 6.0f;

// マウスを1ピクセル動かしたときに回る角度で、単位はラジアン。
static const float kRotateRadiansPerPixel = 0.002f;

DebugCamera::DebugCamera(IInputActions* inputActions, ITime* time, Camera* mainCamera) {
	inputActions_ = inputActions;
	time_ = time;
	mainCamera_ = mainCamera;

	inputActions_->AddAxis2D(kMoveAction);
	inputActions_->BindKeys2D(kMoveAction, Key::W, Key::S, Key::A, Key::D);

	inputActions_->AddButton(kUpAction);
	inputActions_->BindKey(kUpAction, Key::Q);

	inputActions_->AddButton(kDownAction);
	inputActions_->BindKey(kDownAction, Key::E);

	inputActions_->AddAxis2D(kLookAction);
	inputActions_->BindMouseMove(kLookAction);

	inputActions_->AddButton(kLookEnabledAction);
	inputActions_->BindMouseButton(kLookEnabledAction, MouseButton::Right);
}

void DebugCamera::Toggle() {
	if (isAttached_) {
		Detach();
	} else {
		Attach();
	}
}

void DebugCamera::Update() {
	if (!isAttached_) {
		return;
	}

	WorldTransform& transform = GetOwner()->GetTransform();

	// マウスの右を押している間だけ、マウスの動きで向きを変える。
	if (inputActions_->IsHeld(kLookEnabledAction)) {
		const Vector2 look = inputActions_->GetAxis2D(kLookAction);
		transform.rotation_.y += look.x * kRotateRadiansPerPixel;
		transform.rotation_.x += look.y * kRotateRadiansPerPixel;
	}

	// 上や下を向いていても同じ高さを進めるように、Y軸まわりの向きだけで前と右を決める。
	const Vector3 forward = { std::sin(transform.rotation_.y), 0.0f, std::cos(transform.rotation_.y) };
	const Vector3 right = { forward.z, 0.0f, -forward.x };

	// ゲームの時間は止めているので、時間の速さが掛からない秒数で動かす。
	const float distance = kMoveSpeed * time_->GetUnscaledDeltaTime();
	const Vector2 move = inputActions_->GetAxis2D(kMoveAction);
	transform.translation_ += forward * (move.y * distance);
	transform.translation_ += right * (move.x * distance);

	if (inputActions_->IsHeld(kUpAction)) {
		transform.translation_.y += distance;
	}
	if (inputActions_->IsHeld(kDownAction)) {
		transform.translation_.y -= distance;
	}
}

void DebugCamera::SubmitRender(IRenderQueue* renderQueue) {
	if (!isAttached_) {
		return;
	}

	CameraData cameraData = {};
	// カメラの置かれた位置と向きを打ち消すと、世界をカメラから見た形になる。
	cameraData.view = Inverse(GetOwner()->GetTransform().matWorld_);

	// 写る範囲が接続の前後で変わらないように、視野角と距離は元のカメラの値を使う。
	cameraData.fovYRadians = mainCamera_->fovYRadians;
	cameraData.nearZ = mainCamera_->nearZ;
	cameraData.farZ = mainCamera_->farZ;
	renderQueue->SetCamera(cameraData);
}

void DebugCamera::Attach() {
	const WorldTransform& mainTransform = mainCamera_->GetOwner()->GetTransform();
	WorldTransform& transform = GetOwner()->GetTransform();
	transform.translation_ = mainTransform.GetWorldPosition();
	transform.rotation_ = mainTransform.rotation_;

	// 無効にしたコンポーネントは描画を頼まないので、このカメラの絵だけが映る。
	mainCamera_->SetEnabled(false);

	timeScaleBeforeAttach_ = time_->GetTimeScale();
	time_->SetTimeScale(0.0f);

	isAttached_ = true;
	printf("デバッグカメラを接続しました。WASDで移動、QとEで上下、マウスの右を押しながら動かすと向きが変わります。\n");
}

void DebugCamera::Detach() {
	mainCamera_->SetEnabled(true);
	time_->SetTimeScale(timeScaleBeforeAttach_);

	isAttached_ = false;
	printf("デバッグカメラを解除しました。\n");
}

} // namespace KujataEngine
