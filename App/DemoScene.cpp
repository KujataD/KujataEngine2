#include "DemoScene.h"

#include "Scene/Camera.h"
#include "Scene/DirectionalLight.h"
#include "Scene/MeshRenderer.h"

using namespace KujataEngine;

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
/// 指定の位置と大きさで回る立方体を置く。
/// </summary>
static void CreateCube(Scene* scene, MeshHandle mesh, TextureHandle texture, float x, float size, float radiansPerFrame) {
	GameObject* cube = scene->CreateGameObject();
	cube->GetTransform().translation_ = { x, 0.0f, 0.0f };
	cube->GetTransform().scale_ = { size, size, size };
	cube->AddComponent(new MeshRenderer(mesh, texture));
	cube->AddComponent(new Rotator(radiansPerFrame));
}

void BuildDemoScene(Scene* scene, IAssetLibrary* assets) {
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
	GameObject* toriObject = scene->CreateGameObject();
	toriObject->GetTransform().translation_ = { 0.0f, 1.5f, 1.0f };
	toriObject->GetTransform().scale_ = { 0.4f, 0.4f, 0.4f };
	toriObject->AddComponent(new MeshRenderer(tori.mesh, tori.texture));
	toriObject->AddComponent(new Rotator(0.015f));
}
