#include "DemoScene.h"

#include "Scene/Camera.h"
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
static void CreateCube(Scene* scene, float x, float size, float radiansPerFrame) {
	GameObject* cube = scene->CreateGameObject();
	cube->GetTransform().translation_ = { x, 0.0f, 0.0f };
	cube->GetTransform().scale_ = { size, size, size };
	cube->AddComponent(new MeshRenderer());
	cube->AddComponent(new Rotator(radiansPerFrame));
}

void BuildDemoScene(Scene* scene) {
	GameObject* camera = scene->CreateGameObject();
	camera->GetTransform().translation_ = { 0.0f, 2.0f, -6.0f };
	camera->GetTransform().rotation_ = { 0.3f, 0.0f, 0.0f };
	camera->AddComponent(new Camera());

	CreateCube(scene, -2.2f, 1.0f, 0.02f);
	CreateCube(scene, 0.0f, 1.5f, -0.01f);
	CreateCube(scene, 2.2f, 0.7f, 0.04f);
}
