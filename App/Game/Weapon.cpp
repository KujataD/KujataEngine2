#include "Weapon.h"

#include "Bullet.h"
#include "Core/Math/Matrix4x4.h"
#include "SceneGraph/GameObject.h"
#include "Physics/ColliderComponent.h"
#include "Renderer/MeshRenderer.h"

using namespace KujataEngine;

Weapon::Weapon(Scene* scene, ITime* time, MeshHandle bulletMesh, TextureHandle bulletTexture) {
	scene_ = scene;
	time_ = time;
	bulletMesh_ = bulletMesh;
	bulletTexture_ = bulletTexture;
}

void Weapon::Update() {
	if (cooldownSeconds_ > 0.0f) {
		cooldownSeconds_ -= time_->GetDeltaTime();
	}
}

bool Weapon::TryFire(const Vector3& direction) {
	if (cooldownSeconds_ > 0.0f) {
		return false;
	}
	// 余った分を足すと、フレームの長さによらず同じ間隔で撃てる。
	cooldownSeconds_ += interval;

	GameObject* bullet = scene_->CreateGameObject();
	bullet->SetLayer(bulletLayer);
	bullet->GetTransform().translation_ = GetOwner()->GetTransform().translation_ + direction * muzzleDistance;
	bullet->GetTransform().scale_ = {bulletScale, bulletScale, bulletScale};

	MeshRenderer* renderer = new MeshRenderer(bulletMesh_, bulletTexture_);
	renderer->tint = bulletTint;
	bullet->AddComponent(renderer);

	// 重なりだけを知りたいので、トリガーにする。
	SphereColliderComponent* collider = new SphereColliderComponent();
	collider->SetTrigger(true);
	collider->SetCollisionMask(targetLayerMask);
	bullet->AddComponent(collider);

	bullet->AddComponent(new Bullet(time_, direction * bulletSpeed));
	return true;
}
