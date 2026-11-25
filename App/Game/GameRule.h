#pragma once

#include "Health.h"
#include "Interface/IInputActions.h"
#include "SceneGraph/Component.h"
#include "SceneGraph/Scene.h"

#include <functional>

/// <summary>
/// 敵が全滅したら勝ち、自機が倒れたら負けを1回だけ知らせ、デバッグ用のキーでリトライする。
/// </summary>
class GameRule : public KujataEngine::Component {
public:
	/// <summary>
	/// onRetryは、リトライのキーが押されたときに呼ぶ処理。
	/// </summary>
	GameRule(KujataEngine::Scene* scene, KujataEngine::IInputActions* inputActions, Health* playerHealth, std::function<void()> onRetry);

	/// <summary>
	/// リトライに使うアクションを登録する。
	/// </summary>
	static void RegisterActions(KujataEngine::IInputActions* inputActions);

	void Update() override;

private:
	// アクションの名前。
	static constexpr const char* kRetryAction = "Retry";

	/// <summary>
	/// 消す印の付いていない敵が1体でもいればtrueを返す。
	/// </summary>
	bool HasEnemy() const;

	KujataEngine::Scene* scene_;
	KujataEngine::IInputActions* inputActions_;
	Health* playerHealth_;
	std::function<void()> onRetry_;

	// 勝ち負けを知らせた後はtrue。
	bool isFinished_ = false;
};
