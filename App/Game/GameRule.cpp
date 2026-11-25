#include "GameRule.h"

#include "GameLayers.h"

#include <cstdio>

using namespace KujataEngine;

GameRule::GameRule(Scene* scene, IInputActions* inputActions, Health* playerHealth, std::function<void()> onRetry) {
	scene_ = scene;
	inputActions_ = inputActions;
	playerHealth_ = playerHealth;
	onRetry_ = onRetry;
}

void GameRule::RegisterActions(IInputActions* inputActions) {
	inputActions->AddButton(kRetryAction);
	inputActions->BindKey(kRetryAction, Key::I);
}

void GameRule::Update() {
	// デバッグ用のキーなので、決着の前でも受け付ける。
	if (inputActions_->IsPressed(kRetryAction)) {
		onRetry_();
		return;
	}

	if (isFinished_) {
		return;
	}

	// 相打ちは負けにするので、自機を先に見る。
	if (!playerHealth_->IsAlive()) {
		printf("敗北\n");
	} else if (!HasEnemy()) {
		printf("勝利\n");
	} else {
		return;
	}
	isFinished_ = true;
}

bool GameRule::HasEnemy() const {
	// Sceneに名前やタグの検索がないので、Layerで敵を見分ける。
	for (const GameObject* gameObject : scene_->GetGameObjects()) {
		if (gameObject->GetLayer() == GameLayer::kEnemy && !gameObject->IsDestroyed()) {
			return true;
		}
	}
	return false;
}
