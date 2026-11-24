#pragma once

#include "SceneGraph/Scene.h"
#include "Interface/IAssetLibrary.h"
#include "Interface/IConsoleCommands.h"
#include "Interface/IInputActions.h"
#include "Interface/ILineRenderer.h"
#include "Interface/IParticleRenderer.h"
#include "Interface/ITime.h"

/// <summary>
/// 動作確認用に、平らな床と光と、歩いて見回せるカメラと、線の描画を確かめる箱と、崩れては集まる粒子を置き、使うアクションとdebugcameraのコマンドを登録する。
/// </summary>
void BuildDemoScene(KujataEngine::Scene* scene, KujataEngine::IAssetLibrary* assets, KujataEngine::IInputActions* inputActions, KujataEngine::ITime* time, KujataEngine::IConsoleCommands* consoleCommands, KujataEngine::ILineRenderer* lineRenderer, KujataEngine::IParticleRenderer* particleRenderer);
