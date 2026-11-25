#include "GameScene.h"

#include "Core/Math/Matrix4x4.h"
#include "Game/Enemy.h"
#include "Game/GameLayers.h"
#include "Game/MovementFollower.h"
#include "Game/PlayerController.h"
#include "Game/Weapon.h"
#include "Game/Health.h"
#include "Physics/ColliderComponent.h"
#include "Renderer/Camera.h"
#include "Renderer/DirectionalLight.h"
#include "Renderer/MeshRenderer.h"

#include <numbers>

using namespace KujataEngine;

// 床の一辺の長さと厚み。
static constexpr float kFloorSize = 40.0f;
static constexpr float kFloorThickness = 0.2f;

// 自機から見たカメラの位置。
static const Vector3 kCameraOffset = {0.0f, 20.0f, 0.0f};

static const int kEnemyInitHP = 10;
static const int kBulletDamage = 5;

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
/// 弾にだけ当たる敵を置く。
/// </summary>
static void CreateEnemy(Scene* scene, MeshHandle mesh, TextureHandle texture, const Vector3& position) {
	GameObject* enemy = scene->CreateGameObject();
	enemy->SetLayer(GameLayer::kEnemy);
	enemy->GetTransform().translation_ = position;
	enemy->AddComponent(new MeshRenderer(mesh, texture));

	BoxColliderComponent* collider = new BoxColliderComponent();
	collider->SetCollisionMask(GameLayer::ToMask(GameLayer::kPlayerBullet));
	enemy->AddComponent(collider);

	enemy->AddComponent(new Enemy());
	enemy->AddComponent(new Health(10));
}

void BuildGameScene(Scene* scene, IAssetLibrary* assets, IInputActions* inputActions, ITime* time, IWindow* window) {
	PlayerController::RegisterActions(inputActions);
	CreateLight(scene);
	CreateFloor(scene, assets);

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

	// Updateは作った順なので、自機の後に作って追わせる。
	GameObject* camera = scene->CreateGameObject();
	camera->GetTransform().translation_ = player->GetTransform().translation_ + kCameraOffset;
	camera->GetTransform().rotation_ = {std::numbers::pi_v<float> * 0.5f, 0.0f, 0.0f};
	Camera* cameraComponent = new Camera();
	camera->AddComponent(cameraComponent);
	camera->AddComponent(new MovementFollower(&player->GetTransform()));

	Weapon* weapon = new Weapon(scene, time, cubeMesh, assets->GetWhiteTexture());
	weapon->bulletLayer = GameLayer::kPlayerBullet;
	weapon->targetLayerMask = GameLayer::ToMask(GameLayer::kEnemy);
	player->AddComponent(weapon);

	player->AddComponent(new PlayerController(inputActions, time, window, cameraComponent, weapon));
}
