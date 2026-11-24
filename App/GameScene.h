#pragma once

#include "SceneGraph/Scene.h"
#include "Interface/IAssetLibrary.h"
#include "Interface/IInputActions.h"
#include "Interface/ITime.h"
#include "Interface/IWindow.h"

/// <summary>
/// 見下ろし型シューティングのシーンを作る。
/// </summary>
void BuildGameScene(KujataEngine::Scene* scene, KujataEngine::IAssetLibrary* assets, KujataEngine::IInputActions* inputActions, KujataEngine::ITime* time, KujataEngine::IWindow* window);
