#pragma once

#include "Framework/Scene.h"
#include "Interface/IAssetLibrary.h"
#include "Interface/IConsoleCommands.h"
#include "Interface/IInputActions.h"
#include "Interface/ITime.h"

/// <summary>
/// 動作確認用に、平らな床と光と、歩いて見回せるカメラを置き、使うアクションとdebugcameraのコマンドを登録する。
/// </summary>
void BuildDemoScene(KujataEngine::Scene* scene, KujataEngine::IAssetLibrary* assets, KujataEngine::IInputActions* inputActions, KujataEngine::ITime* time, KujataEngine::IConsoleCommands* consoleCommands);
