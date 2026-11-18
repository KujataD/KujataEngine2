#include "DemoScene.h"

#include "Interface/LineShapes.h"
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

// 線の描画を確かめる箱の位置で、目の正面の少し先に置く。
static const Vector3 kLineCheckBoxPosition = {0.0f, 0.5f, 6.0f};

/// <summary>
/// 付いている物の箱に重ねて線の箱を描き、床の格子と球と太い線も描いて、線の描画を確かめる。
/// </summary>
class LineShowcase : public Component {
public:
	LineShowcase(ILineRenderer* lineRenderer) {
		lineRenderer_ = lineRenderer;
	}

	void Update() override {
#ifdef USE_IMGUI
		const Vector4 magenta = {1.0f, 0.2f, 0.9f, 1.0f};
		const Vector4 faintGray = {0.6f, 0.6f, 0.6f, 0.35f};
		const Vector4 cyan = {0.2f, 0.9f, 1.0f, 1.0f};
		const Vector4 red = {1.0f, 0.25f, 0.2f, 1.0f};
		const Vector4 green = {0.3f, 1.0f, 0.4f, 1.0f};
		const Vector4 yellow = {1.0f, 0.9f, 0.2f, 1.0f};

		// MeshRendererの立方体と同じ行列を渡すので、メッシュの辺にぴったり重なる。
		SubmitLineBox(*lineRenderer_, GetOwner()->GetTransform().matWorld_, magenta);

		// 床に1マス1の格子を、床の面より少しだけ上に描く。
		const Matrix4x4 gridWorld = MakeAffineMatrix({kFloorSize, 1.0f, kFloorSize}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.01f, 0.0f});
		SubmitLineGrid(*lineRenderer_, gridWorld, faintGray, static_cast<int>(kFloorSize), static_cast<int>(kFloorSize));

		const Matrix4x4 sphereWorld = MakeAffineMatrix({1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {2.0f, 1.0f, 6.0f});
		SubmitLineSphere(*lineRenderer_, sphereWorld, cyan);

		// ワールドの長さで太い線と、ピクセルで太い線。
		lineRenderer_->SubmitThickLine({-2.5f, 0.0f, 6.0f}, {-2.5f, 2.0f, 6.0f}, red, 0.1f, LineWidthUnit::World);
		lineRenderer_->SubmitThickLine({-2.0f, 0.2f, 8.0f}, {3.0f, 0.2f, 8.0f}, green, 4.0f, LineWidthUnit::Pixel);

		// 箱の中を通る線を、深度テストなしで描き、箱に隠れずに見えることを確かめる。
		lineRenderer_->SubmitLine({-1.0f, 0.5f, 6.0f}, {1.0f, 0.5f, 6.0f}, yellow, false);
#endif // USE_IMGUI
	}

private:
	ILineRenderer* lineRenderer_;
};

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
/// 線の描画を確かめるために、一辺1の箱を置き、線の箱と格子などを毎フレーム重ねる。
/// </summary>
static void CreateLineCheckBox(Scene* scene, IAssetLibrary* assets, ILineRenderer* lineRenderer) {
	GameObject* box = scene->CreateGameObject();
	box->GetTransform().translation_ = kLineCheckBoxPosition;
	box->AddComponent(new MeshRenderer(assets->GetCubeMesh(), assets->GetWhiteTexture()));
	box->AddComponent(new LineShowcase(lineRenderer));
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

void BuildDemoScene(Scene* scene, IAssetLibrary* assets, IInputActions* inputActions, ITime* time, IConsoleCommands* consoleCommands, ILineRenderer* lineRenderer) {
	RegisterEyeActions(inputActions);
	CreateLight(scene);
	CreateFloor(scene, assets);
	CreateLineCheckBox(scene, assets, lineRenderer);

	Camera* camera = CreateEye(scene, inputActions, time);
	CreateDebugCamera(scene, inputActions, time, consoleCommands, camera);
}
