#pragma once

#include "Framework/Scene.h"
#include "Interface/IAssetLibrary.h"
#include "Interface/IInputActions.h"
#include "Interface/ITime.h"
#include "Interface/IWindow.h"

/// <summary>
/// 動作確認用に、カメラと光と敵3体と、撃った球が通ると形になる点群と、マウスの方へ撃てる自機を置き、使うアクションを登録する。
/// </summary>
void BuildDemoScene(KujataEngine::Scene* scene, KujataEngine::IAssetLibrary* assets, KujataEngine::IInputActions* inputActions, KujataEngine::ITime* time, KujataEngine::IWindow* window);
