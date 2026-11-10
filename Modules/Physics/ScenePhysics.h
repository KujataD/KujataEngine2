#pragma once

namespace KujataEngine {

class Scene;
class ColliderComponent;
struct Contact;

/// <summary>
/// 衝突の検出はSceneCollisionSystemに任せ、Rigidbodyの重力つきの移動と衝突への応答だけを扱う。
/// </summary>
namespace ScenePhysics {

/// <summary>
/// Rigidbodyの速度で位置を進め、動的なものには重力を足してFreeze Positionの軸を止める。
/// </summary>
void IntegrateRigidbodies(Scene& scene, float deltaTime);

/// <summary>
/// AからBへ向かう法線の接触情報で、非トリガーの交差ペアを押し出して速度を反射する。
/// </summary>
void ResolveCollisionResponse(ColliderComponent* colliderA, ColliderComponent* colliderB, const Contact& contact);

} // namespace ScenePhysics

} // namespace KujataEngine
