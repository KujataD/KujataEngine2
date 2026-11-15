#pragma once

#include "Framework/Scene.h"
#include "Interface/IAssetLibrary.h"
#include "Interface/IConsoleCommands.h"
#include "Interface/IInputActions.h"
#include "Interface/ITime.h"
#include "Interface/IWindow.h"

/// <summary>
/// 動作確認用に、目になるカメラと、点の球で崩せる点の立方体5個を置き、使うアクションとdebugcameraのコマンドを登録する。
/// </summary>
void BuildDemoScene(KujataEngine::Scene* scene, KujataEngine::IAssetLibrary* assets, KujataEngine::IInputActions* inputActions, KujataEngine::ITime* time, KujataEngine::IWindow* window, KujataEngine::IConsoleCommands* consoleCommands);
