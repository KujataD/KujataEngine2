#include "DemoScene.h"

#include "Scene/Camera.h"
#include "Scene/DebugCamera.h"
#include "Scene/DirectionalLight.h"
#include "Scene/MeshRenderer.h"

#include <algorithm>
#include <cmath>

using namespace KujataEngine;

// アクションの名前。
static const char* const kMoveAction = "Move";
static const char* const kLookAction = "Look";
static const char* const kLookEnabledAction = "LookEnabled";

// 目の高さと、1秒に歩く距離。
static const float kEyeHeight = 1.6f;
static const float kWalkSpeed = 4.0f;

// マウスを1ピクセル動かしたときに回る角度で、単位はラジアン。
static const float kLookRadiansPerPixel = 0.003f;

// 真上と真下を越えて裏返らないようにする、上下の向きの限界で、単位はラジアン。
static const float kMaxPitch = 1.4f;

// 床の一辺の長さと厚みで、上の面が高さ0になるように置く。
static const float kFloorSize = 40.0f;
static const float kFloorThickness = 0.2f;

/// <summary>
/// 目になるカメラを、歩かせて見回させる。
/// </summary>
class EyeController : public Component {
public:
	EyeController(IInputActions* inputActions, ITime* time, Camera* camera) {
		inputActions_ = inputActions;
		time_ = time;
		camera_ = camera;
	}

	void Update() override {
		// デバッグカメラの接続中はカメラが無効になるので、その間は同じキーで目が動かないようにする。
		if (!camera_->IsEnabled()) {
			return;
		}

		WorldTransform& transform = GetOwner()->GetTransform();

		// マウスの右を押している間だけ、マウスの動きで見回す。
		if (inputActions_->IsHeld(kLookEnabledAction)) {
			const Vector2 look = inputActions_->GetAxis2D(kLookAction);
			transform.rotation_.y += look.x * kLookRadiansPerPixel;
			transform.rotation_.x = std::clamp(transform.rotation_.x + look.y * kLookRadiansPerPixel, -kMaxPitch, kMaxPitch);
		}

		// 上や下を向いていても同じ高さを歩くように、Y軸まわりの向きだけで前と右を決める。
		const Vector3 forward = {std::sin(transform.rotation_.y), 0.0f, std::cos(transform.rotation_.y)};
		const Vector3 right = {forward.z, 0.0f, -forward.x};
		const Vector2 move = inputActions_->GetAxis2D(kMoveAction);
		const float distance = kWalkSpeed * time_->GetDeltaTime();
		transform.translation_ += forward * (move.y * distance);
		transform.translation_ += right * (move.x * distance);
	}

private:
	IInputActions* inputActions_;
	ITime* time_;
	Camera* camera_;
};

/// <summary>
/// 歩く、見回すアクションを登録する。
/// </summary>
static void RegisterEyeActions(IInputActions* inputActions) {
	inputActions->AddAxis2D(kMoveAction);
	inputActions->BindKeys2D(kMoveAction, Key::W, Key::S, Key::A, Key::D);
	inputActions->BindKeys2D(kMoveAction, Key::Up, Key::Down, Key::Left, Key::Right);
	inputActions->BindPadStick(kMoveAction, PadStick::Left);

	inputActions->AddAxis2D(kLookAction);
	inputActions->BindMouseMove(kLookAction);

	inputActions->AddButton(kLookEnabledAction);
	inputActions->BindMouseButton(kLookEnabledAction, MouseButton::Right);
}

/// <summary>
/// 光がないというログを出さないための光を置く。
/// </summary>
static void CreateLight(Scene* scene) {
	GameObject* light = scene->CreateGameObject();
	light->GetTransform().rotation_ = {0.9f, -0.5f, 0.0f};
	light->AddComponent(new DirectionalLight());
}

/// <summary>
/// 平らなアリーナの床として、薄く広い立方体を置く。
/// </summary>
static void CreateFloor(Scene* scene, IAssetLibrary* assets) {
	GameObject* floor = scene->CreateGameObject();
	floor->GetTransform().translation_ = {0.0f, -kFloorThickness * 0.5f, 0.0f};
	floor->GetTransform().scale_ = {kFloorSize, kFloorThickness, kFloorSize};
	floor->AddComponent(new MeshRenderer(assets->GetCubeMesh(), assets->GetWhiteTexture()));
}

/// <summary>
/// 目になるカメラを置く。
/// </summary>
static Camera* CreateEye(Scene* scene, IInputActions* inputActions, ITime* time) {
	Camera* camera = new Camera();

	GameObject* eye = scene->CreateGameObject();
	eye->GetTransform().translation_ = {0.0f, kEyeHeight, 0.0f};
	eye->AddComponent(camera);
	eye->AddComponent(new EyeController(inputActions, time, camera));
	return camera;
}

/// <summary>
/// ターミナルのdebugcameraで切り替わるデバッグカメラを置く。
/// </summary>
static void CreateDebugCamera(Scene* scene, IInputActions* inputActions, ITime* time, IConsoleCommands* consoleCommands, Camera* mainCamera) {
	DebugCamera* debugCamera = new DebugCamera(inputActions, time, mainCamera);
	scene->CreateGameObject()->AddComponent(debugCamera);

	consoleCommands->AddCommand("debugcamera", [debugCamera]() {
		debugCamera->Toggle();
	});
}

void BuildDemoScene(Scene* scene, IAssetLibrary* assets, IInputActions* inputActions, ITime* time, IConsoleCommands* consoleCommands) {
	RegisterEyeActions(inputActions);
	CreateLight(scene);
	CreateFloor(scene, assets);

	Camera* camera = CreateEye(scene, inputActions, time);
	CreateDebugCamera(scene, inputActions, time, consoleCommands, camera);
}
