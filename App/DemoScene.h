#pragma once

#include "Framework/Scene.h"
#include "Interface/IAssetLibrary.h"
#include "Interface/IInputActions.h"

/// <summary>
/// 動作確認用に、カメラと光と回る立方体3個と操作できるモデルを置き、使うアクションを登録する。
/// </summary>
void BuildDemoScene(KujataEngine::Scene* scene, KujataEngine::IAssetLibrary* assets, KujataEngine::IInputActions* inputActions);
