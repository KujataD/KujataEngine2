#include "DemoScene.h"

#include "Scene/Camera.h"
#include "Scene/DirectionalLight.h"
#include "Scene/MeshRenderer.h"

using namespace KujataEngine;

// デモで使うアクションの名前。
static const char* const kMoveAction = "Move";
static const char* const kResetPositionAction = "ResetPosition";
static const char* const kLookAction = "Look";
static const char* const kLookEnabledAction = "LookEnabled";

// 1フレームに進む距離で、秒あたりにするのはゲームの時間を作るときに行う。
static const float kMoveDistancePerFrame = 0.03f;

// マウスを1ピクセル動かしたときに回る角度で、単位はラジアン。
static const float kTurnRadiansPerPixel = 0.01f;

/// <summary>
/// 付いているGameObjectをY軸まわりに回し続ける動作確認用のコンポーネント。
/// </summary>
class Rotator : public Component {
public:
	Rotator(float radiansPerFrame) {
		radiansPerFrame_ = radiansPerFrame;
	}

	void Update() override {
		GetOwner()->GetTransform().rotation_.y += radiansPerFrame_;
	}

private:
	float radiansPerFrame_;
};

/// <summary>
/// アクションに合わせて、付いているGameObjectを動かしたり回したりする動作確認用のコンポーネント。
/// </summary>
class ToriController : public Component {
public:
	ToriController(IInputActions* inputActions, const Vector3& startPosition) {
		inputActions_ = inputActions;
		startPosition_ = startPosition;
	}

	void Update() override {
		WorldTransform& transform = GetOwner()->GetTransform();

		// 2軸の上を奥の向きにして、床の上を動かす。
		const Vector2 move = inputActions_->GetAxis2D(kMoveAction);
		transform.translation_.x += move.x * kMoveDistancePerFrame;
		transform.translation_.z += move.y * kMoveDistancePerFrame;

		if (inputActions_->IsPressed(kResetPositionAction)) {
			transform.translation_ = startPosition_;
		}

		// マウスの右を押している間だけ、左右の動きで向きを変える。
		if (inputActions_->IsHeld(kLookEnabledAction)) {
			transform.rotation_.y += inputActions_->GetAxis2D(kLookAction).x * kTurnRadiansPerPixel;
		}
	}

private:
	IInputActions* inputActions_;
	Vector3 startPosition_;
};

/// <summary>
/// デモで使うアクションと、その割り当てを登録する。
/// </summary>
static void RegisterDemoActions(IInputActions* inputActions) {
	inputActions->AddAxis2D(kMoveAction);
	inputActions->BindKeys2D(kMoveAction, Key::W, Key::S, Key::A, Key::D);
	inputActions->BindKeys2D(kMoveAction, Key::Up, Key::Down, Key::Left, Key::Right);
	inputActions->BindPadStick(kMoveAction, PadStick::Left);

	inputActions->AddButton(kResetPositionAction);
	inputActions->BindKey(kResetPositionAction, Key::Space);
	inputActions->BindPadButton(kResetPositionAction, PadButton::A);
	inputActions->BindMouseButton(kResetPositionAction, MouseButton::Left);

	inputActions->AddAxis2D(kLookAction);
	inputActions->BindMouseMove(kLookAction);

	inputActions->AddButton(kLookEnabledAction);
	inputActions->BindMouseButton(kLookEnabledAction, MouseButton::Right);
}

/// <summary>
/// 指定の位置と大きさで回る立方体を置く。
/// </summary>
static void CreateCube(Scene* scene, MeshHandle mesh, TextureHandle texture, float x, float size, float radiansPerFrame) {
	GameObject* cube = scene->CreateGameObject();
	cube->GetTransform().translation_ = { x, 0.0f, 0.0f };
	cube->GetTransform().scale_ = { size, size, size };
	cube->AddComponent(new MeshRenderer(mesh, texture));
	cube->AddComponent(new Rotator(radiansPerFrame));
}

void BuildDemoScene(Scene* scene, IAssetLibrary* assets, IInputActions* inputActions) {
	RegisterDemoActions(inputActions);

	GameObject* camera = scene->CreateGameObject();
	camera->GetTransform().translation_ = { 0.0f, 2.0f, -6.0f };
	camera->GetTransform().rotation_ = { 0.3f, 0.0f, 0.0f };
	camera->AddComponent(new Camera());

	GameObject* light = scene->CreateGameObject();
	// 右上の手前から斜めに当たるように傾ける。
	light->GetTransform().rotation_ = { 0.9f, -0.5f, 0.0f };
	light->AddComponent(new DirectionalLight());

	const MeshHandle cubeMesh = assets->GetCubeMesh();
	const TextureHandle uvChecker = assets->LoadTexture("Textures/uvChecker.png");
	CreateCube(scene, cubeMesh, uvChecker, -2.2f, 1.0f, 0.02f);
	CreateCube(scene, cubeMesh, uvChecker, 0.0f, 1.5f, -0.01f);
	CreateCube(scene, cubeMesh, uvChecker, 2.2f, 0.7f, 0.04f);

	const ModelHandles tori = assets->LoadModel("Models/tori/tori.obj");
	const Vector3 toriStartPosition = { 0.0f, 1.5f, 1.0f };
	GameObject* toriObject = scene->CreateGameObject();
	toriObject->GetTransform().translation_ = toriStartPosition;
	toriObject->GetTransform().scale_ = { 0.4f, 0.4f, 0.4f };
	toriObject->AddComponent(new MeshRenderer(tori.mesh, tori.texture));
	toriObject->AddComponent(new ToriController(inputActions, toriStartPosition));
}
