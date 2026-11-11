#include "DemoScene.h"

#include "Interface/ICollisionListener.h"
#include "Physics/ColliderComponent.h"
#include "Scene/Camera.h"
#include "Scene/DirectionalLight.h"
#include "Scene/MeshRenderer.h"

#include <cmath>
using namespace KujataEngine;

// アクションの名前。
static const char* const kMoveAction = "Move";
static const char* const kFireAction = "Fire";
static const char* const kLookAction = "Look";
static const char* const kLookEnabledAction = "LookEnabled";

// 衝突の絞り込みに使うLayerの番号。
static const uint32_t kBulletLayer = 2;
static const uint32_t kEnemyLayer = 3;

// 自機が1秒に進む距離。
static const float kPlayerMoveSpeed = 3.0f;

// 自機から見たカメラの位置で、真上から見下ろす。
static const Vector3 kCameraOffset = {0.0f, 20.0f, 0.0f};

// マウスを1ピクセル動かしたときに回る角度で、単位はラジアン。
static const float kTurnRadiansPerPixel = 0.01f;

// 球が1秒に進む距離。
static const float kBulletSpeed = 8.0f;

// 球を撃ってから消えるまでの秒数。
static const float kBulletLifeSeconds = 2.0f;

// 球の見た目の大きさで、当たり判定の半径にも掛かる。
static const float kBulletScale = 0.2f;

// マウスの指す点が自機にこれより近いと向きが決まらないので、前へ撃つ。
static const float kMinAimDistance = 0.01f;

/// <summary>
/// まっすぐ飛び、決まった秒数がたつと消える球。
/// </summary>
class Bullet : public Component {
public:
	Bullet(ITime* time, const Vector3& velocity) {
		time_ = time;
		velocity_ = velocity;
	}
 
	void Update() override {
		const float deltaTime = time_->GetDeltaTime();
		GetOwner()->GetTransform().translation_ += velocity_ * deltaTime;

		// 何にも当たらなかった球が残り続けないように、時間で消す。
		aliveSeconds_ += deltaTime;
		if (aliveSeconds_ >= kBulletLifeSeconds) {
			GetOwner()->Destroy();
		}
	}

private:
	ITime* time_;
	Vector3 velocity_;
	float aliveSeconds_ = 0.0f;
};

/// <summary>
/// 親のない相手が動いた分だけ自分も動かし、相手との位置の差を保つ。
/// </summary>
class MovementFollower : public Component {
public:
	MovementFollower(const WorldTransform* target) {
		target_ = target;
	}

	void Start() override {
		previousTargetPosition_ = target_->translation_;
	}

	void Update() override {
		// 親子にすると相手の回転や縮小まで付いてくるので、位置の変化だけを足す。
		const Vector3 movement = target_->translation_ - previousTargetPosition_;
		GetOwner()->GetTransform().translation_ += movement;
		previousTargetPosition_ = target_->translation_;
	}

private:
	const WorldTransform* target_;
	Vector3 previousTargetPosition_;
};

/// <summary>
/// 球に当たると、自分と球を消す敵。
/// </summary>
class Enemy : public Component, public ICollisionListener {
public:
	void OnTriggerEnter(GameObject* other) override {
		// 消える途中の物は、同じフレームの物理でもう当たっているので数えない。
		if (GetOwner()->IsDestroyed() || other->IsDestroyed()) {
			return;
		}
		other->Destroy();
		GetOwner()->Destroy();
	}
};

/// <summary>
/// アクションで動き、撃つアクションでマウスが指す方へ球を撃つ自機。
/// </summary>
class PlayerController : public Component {
public:
	PlayerController(Scene* scene, IInputActions* inputActions, ITime* time, IWindow* window, Camera* camera, MeshHandle bulletMesh, TextureHandle bulletTexture) {
		scene_ = scene;
		inputActions_ = inputActions;
		time_ = time;
		window_ = window;
		camera_ = camera;
		bulletMesh_ = bulletMesh;
		bulletTexture_ = bulletTexture;
	}

	void Update() override {
		WorldTransform& transform = GetOwner()->GetTransform();

		// 2軸の上を奥の向きにして、床の上を動かす。
		const Vector2 move = inputActions_->GetAxis2D(kMoveAction);
		const float distance = kPlayerMoveSpeed * time_->GetDeltaTime();
		transform.translation_.x += move.x * distance;
		transform.translation_.z += move.y * distance;

		// マウスの右を押している間だけ、左右の動きで向きを変える。
		if (inputActions_->IsHeld(kLookEnabledAction)) {
			transform.rotation_.y += inputActions_->GetAxis2D(kLookAction).x * kTurnRadiansPerPixel;
		}

		if (inputActions_->IsPressed(kFireAction)) {
			SpawnBullet();
		}
	}

private:
	/// <summary>
	/// 自機の位置に球を作り、マウスが指す方へ飛ばす。
	/// </summary>
	void SpawnBullet() {
		const WorldTransform& playerTransform = GetOwner()->GetTransform();
		const Vector3 direction = ComputeAimDirection(playerTransform.translation_);

		GameObject* bullet = scene_->CreateGameObject();
		bullet->SetLayer(kBulletLayer);
		bullet->GetTransform().translation_ = playerTransform.translation_;
		bullet->GetTransform().scale_ = {kBulletScale, kBulletScale, kBulletScale};
		bullet->AddComponent(new MeshRenderer(bulletMesh_, bulletTexture_));

		// 押し出しは要らず重なりだけを知りたいので、トリガーにする。
		SphereColliderComponent* collider = new SphereColliderComponent();
		collider->SetTrigger(true);
		collider->SetCollisionMask(1u << kEnemyLayer);
		bullet->AddComponent(collider);

		bullet->AddComponent(new Bullet(time_, direction * kBulletSpeed));
	}

	/// <summary>
	/// 自機から、マウスが指す点へ向かう水平な向きを返し、決まらないときは自機の前を返す。
	/// </summary>
	Vector3 ComputeAimDirection(const Vector3& playerPosition) {
		// Y軸まわりの向きだけを使うので、上下には飛ばない。
		const Vector3 forward = TransformNormal({0.0f, 0.0f, 1.0f}, MakeRotateYMatrix(GetOwner()->GetTransform().rotation_.y));

		// 自機と同じ高さの面で拾うと、球が上下せずにマウスの指す点を通る。
		Vector3 target;
		if (!PickHorizontalPlane(inputActions_->GetMousePosition(), playerPosition.y, &target)) {
			return forward;
		}

		Vector3 toTarget = target - playerPosition;
		toTarget.y = 0.0f;
		if (Length(toTarget) < kMinAimDistance) {
			return forward;
		}
		return Normalize(toTarget);
	}

	/// <summary>
	/// 画面の点からカメラの奥へ伸ばした線が、高さplaneYの水平な面と交わる点を求める。
	/// </summary>
	bool PickHorizontalPlane(const Vector2& screenPosition, float planeY, Vector3* hitPosition) {
		const float width = static_cast<float>(window_->GetClientWidth());
		const float height = static_cast<float>(window_->GetClientHeight());
		// 最小化中は大きさが0になり、縦横比が求まらない。
		if (width <= 0.0f || height <= 0.0f) {
			return false;
		}

		// カメラの行列は描画の直前に更新されるので、ここでは今映っている絵のものになる。
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

		// 負になるのは、面がカメラの後ろにあるとき。
		const float t = (planeY - nearPoint.y) / ray.y;
		if (t < 0.0f) {
			return false;
		}

		*hitPosition = nearPoint + ray * t;
		return true;
	}

	Scene* scene_;
	IInputActions* inputActions_;
	ITime* time_;
	IWindow* window_;
	Camera* camera_;
	MeshHandle bulletMesh_;
	TextureHandle bulletTexture_;
};

/// <summary>
/// 自機の操作に使うアクションと、その割り当てを登録する。
/// </summary>
static void RegisterPlayerActions(IInputActions* inputActions) {
	inputActions->AddAxis2D(kMoveAction);
	inputActions->BindKeys2D(kMoveAction, Key::W, Key::S, Key::A, Key::D);
	inputActions->BindKeys2D(kMoveAction, Key::Up, Key::Down, Key::Left, Key::Right);
	inputActions->BindPadStick(kMoveAction, PadStick::Left);

	inputActions->AddButton(kFireAction);
	inputActions->BindKey(kFireAction, Key::Space);
	inputActions->BindPadButton(kFireAction, PadButton::A);
	inputActions->BindMouseButton(kFireAction, MouseButton::Left);

	inputActions->AddAxis2D(kLookAction);
	inputActions->BindMouseMove(kLookAction);

	inputActions->AddButton(kLookEnabledAction);
	inputActions->BindMouseButton(kLookEnabledAction, MouseButton::Right);
}

/// <summary>
/// 指定の位置に、球にだけ当たる敵を置く。
/// </summary>
static void CreateEnemy(Scene* scene, MeshHandle mesh, TextureHandle texture, const Vector3& position) {
	GameObject* enemy = scene->CreateGameObject();
	enemy->SetLayer(kEnemyLayer);
	enemy->GetTransform().translation_ = position;
	enemy->AddComponent(new MeshRenderer(mesh, texture));

	BoxColliderComponent* collider = new BoxColliderComponent();
	collider->SetCollisionMask(1u << kBulletLayer);
	enemy->AddComponent(collider);

	enemy->AddComponent(new Enemy());
}

void BuildDemoScene(Scene* scene, IAssetLibrary* assets, IInputActions* inputActions, ITime* time, IWindow* window) {
	RegisterPlayerActions(inputActions);

	GameObject* light = scene->CreateGameObject();
	// 右上の手前から斜めに当たるように傾ける。
	light->GetTransform().rotation_ = {0.9f, -0.5f, 0.0f};
	light->AddComponent(new DirectionalLight());

	const MeshHandle cubeMesh = assets->GetCubeMesh();
	const TextureHandle uvChecker = assets->LoadTexture("Textures/uvChecker.png");
	CreateEnemy(scene, cubeMesh, uvChecker, {-2.5f, 0.5f, 4.0f});
	CreateEnemy(scene, cubeMesh, uvChecker, {0.0f, 0.5f, 4.0f});
	CreateEnemy(scene, cubeMesh, uvChecker, {2.5f, 0.5f, 4.0f});

	const ModelHandles tori = assets->LoadModel("Models/tori/tori.obj");
	GameObject* player = scene->CreateGameObject();
	player->GetTransform().translation_ = {0.0f, 0.5f, -1.0f};
	player->GetTransform().scale_ = {0.4f, 0.4f, 0.4f};
	player->AddComponent(new MeshRenderer(tori.mesh, tori.texture));

	// Updateは作った順に呼ばれるので、自機より後に作り、自機が動いた後に追わせる。
	GameObject* camera = scene->CreateGameObject();
	camera->GetTransform().translation_ = player->GetTransform().translation_ + kCameraOffset;
	camera->GetTransform().rotation_ = {std::numbers::pi_v<float> * 0.5f, 0.0f, 0.0f};
	Camera* cameraComponent = new Camera();
	camera->AddComponent(cameraComponent);
	camera->AddComponent(new MovementFollower(&player->GetTransform()));

	player->AddComponent(new PlayerController(scene, inputActions, time, window, cameraComponent, cubeMesh, assets->GetWhiteTexture()));
}