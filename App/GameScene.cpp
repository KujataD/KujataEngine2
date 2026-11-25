#include "GameScene.h"

#include "Core/Math/Matrix4x4.h"
#include "Game/Enemy.h"
#include "Game/GameLayers.h"
#include "Game/GameRule.h"
#include "Game/Health.h"
#include "Game/MovementFollower.h"
#include "Game/PlayerController.h"
#include "Game/Weapon.h"
#include "Physics/ColliderComponent.h"
#include "Physics/RigidbodyComponent.h"
#include "Renderer/Camera.h"
#include "Renderer/DirectionalLight.h"
#include "Renderer/MeshRenderer.h"

#include <numbers>

using namespace KujataEngine;

// ----------------------------------------------
// 調整項目
// ----------------------------------------------

// 床の一辺の長さと厚み。
static constexpr float kFloorSize = 40.0f;
static constexpr float kFloorThickness = 0.2f;

// 自機から見たカメラの位置。
static const Vector3 kCameraOffset = {0.0f, 20.0f, 0.0f};

// 敵と自機の最初のHP。
static const int kEnemyInitHP = 10;
static const int kPlayerInitHP = 10;

// 自機の弾と敵の弾が1発で減らすHP。
static const int kBulletDamage = 2;
static const int kEnemyBulletDamage = 2;

// 敵が撃つ間隔で、単位は秒。
static constexpr float kEnemyFireInterval = 1.0f;

// 敵の弾が1秒に進む距離。
static constexpr float kEnemyBulletSpeed = 4.0f;

// 敵の中心から弾を出す位置までの距離で、敵の箱の外に出す。
static constexpr float kEnemyMuzzleDistance = 0.7f;

// 敵の弾の色。
static const Vector4 kEnemyBulletTint = {0.6f, 0.3f, 1.0f, 1.0f};

// ----------------------------------------------
// オブジェクト生成
// ----------------------------------------------

/// <summary>
/// 斜めから当たる光を置く。
/// </summary>
static void CreateLight(Scene* scene) {
	GameObject* light = scene->CreateGameObject();
	light->GetTransform().rotation_ = {0.9f, -0.5f, 0.0f};
	light->AddComponent(new DirectionalLight());
}

/// <summary>
/// 床として薄く広い立方体を置く。
/// </summary>
static void CreateFloor(Scene* scene, IAssetLibrary* assets) {
	GameObject* floor = scene->CreateGameObject();
	floor->GetTransform().translation_ = {0.0f, -kFloorThickness * 0.5f, 0.0f};
	floor->GetTransform().scale_ = {kFloorSize, kFloorThickness, kFloorSize};
	floor->AddComponent(new MeshRenderer(assets->GetCubeMesh(), assets->GetWhiteTexture()));
}

/// <summary>
/// 自機の弾とほかの敵に当たり、targetへ揺れながら近づいて撃つ敵を置く。
/// </summary>
static void CreateEnemy(Scene* scene, IAssetLibrary* assets, ITime* time, TextureHandle texture, const Vector3& position, GameObject* target, float noiseSeed = 0.0f) {
	GameObject* enemy = scene->CreateGameObject();
	enemy->SetLayer(GameLayer::kEnemy);
	enemy->GetTransform().translation_ = position;
	enemy->AddComponent(new MeshRenderer(assets->GetCubeMesh(), texture));

	BoxColliderComponent* collider = new BoxColliderComponent();
	collider->SetCollisionMask(GameLayer::ToMask(GameLayer::kPlayerBullet) | GameLayer::ToMask(GameLayer::kEnemy));
	enemy->AddComponent(collider);

	// 敵どうしが重ならないように押し出させ、床に当たり判定がないので重力は切る。
	RigidbodyComponent* rigidbody = new RigidbodyComponent();
	rigidbody->SetGravityScale(0.0f);
	enemy->AddComponent(rigidbody);

	Weapon* weapon = new Weapon(scene, time, assets->GetCubeMesh(), assets->GetWhiteTexture());
	weapon->interval = kEnemyFireInterval;
	weapon->bulletSpeed = kEnemyBulletSpeed;
	weapon->bulletDamage = kEnemyBulletDamage;
	weapon->bulletTint = kEnemyBulletTint;
	weapon->muzzleDistance = kEnemyMuzzleDistance;
	weapon->bulletLayer = GameLayer::kEnemyBullet;
	weapon->targetLayerMask = GameLayer::ToMask(GameLayer::kPlayer);
	enemy->AddComponent(weapon);

	enemy->AddComponent(new Enemy(time, weapon, target, noiseSeed));
	enemy->AddComponent(new Health(kEnemyInitHP));
}

/// <summary>
/// ゲームの物を最初の状態で置き、リトライのたびに呼び直す。
/// </summary>
static void PlaceGameObjects(Scene* scene, IAssetLibrary* assets, IInputActions* inputActions, ITime* time, IWindow* window) {
	const TextureHandle uvChecker = assets->LoadTexture("Textures/uvChecker.png");
	const ModelHandles tori = assets->LoadModel("Models/tori/tori.obj");

	CreateLight(scene);
	CreateFloor(scene, assets);

	GameObject* player = scene->CreateGameObject();
	player->SetLayer(GameLayer::kPlayer);
	player->GetTransform().translation_ = {0.0f, 0.5f, -1.0f};
	player->GetTransform().scale_ = {0.4f, 0.4f, 0.4f};
	player->AddComponent(new MeshRenderer(tori.mesh, tori.texture));

	BoxColliderComponent* playerCollider = new BoxColliderComponent();
	playerCollider->SetCollisionMask(GameLayer::ToMask(GameLayer::kEnemyBullet));
	player->AddComponent(playerCollider);

	Health* playerHealth = new Health(kPlayerInitHP);
	player->AddComponent(playerHealth);

	// Updateは作った順なので、自機の後に作って追わせる。
	GameObject* camera = scene->CreateGameObject();
	camera->GetTransform().translation_ = player->GetTransform().translation_ + kCameraOffset;
	camera->GetTransform().rotation_ = {std::numbers::pi_v<float> * 0.5f, 0.0f, 0.0f};
	Camera* cameraComponent = new Camera();
	camera->AddComponent(cameraComponent);
	camera->AddComponent(new MovementFollower(&player->GetTransform()));

	Weapon* weapon = new Weapon(scene, time, assets->GetCubeMesh(), assets->GetWhiteTexture());
	weapon->bulletDamage = kBulletDamage;
	weapon->bulletLayer = GameLayer::kPlayerBullet;
	weapon->targetLayerMask = GameLayer::ToMask(GameLayer::kEnemy);
	player->AddComponent(weapon);

	player->AddComponent(new PlayerController(inputActions, time, window, cameraComponent, weapon));

	// 敵は狙う相手として自機を受け取るので、自機の後に作る。
	CreateEnemy(scene, assets, time, uvChecker, {-2.5f, 0.5f, 4.0f}, player, 0.0f);
	CreateEnemy(scene, assets, time, uvChecker, {0.0f, 0.5f, 4.0f}, player, -10.0f);
	CreateEnemy(scene, assets, time, uvChecker, {2.5f, 0.5f, 4.0f}, player, 10.0f);

	GameObject* gameRule = scene->CreateGameObject();
	gameRule->AddComponent(new GameRule(scene, inputActions, playerHealth, [scene, assets, inputActions, time, window]() {
		// 印を付けた物はフレームの終わりに消え、置き直した物が次のフレームから動く。
		for (GameObject* gameObject : scene->GetGameObjects()) {
			gameObject->Destroy();
		}
		PlaceGameObjects(scene, assets, inputActions, time, window);
	}));
}

void BuildGameScene(Scene* scene, IAssetLibrary* assets, IInputActions* inputActions, ITime* time, IWindow* window) {
	// 置き直すたびに登録すると割り当てが重なるので、アクションの登録は1回だけにする。
	PlayerController::RegisterActions(inputActions);
	GameRule::RegisterActions(inputActions);
	PlaceGameObjects(scene, assets, inputActions, time, window);
}
