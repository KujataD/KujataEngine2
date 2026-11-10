#pragma once

#include "Framework/Scene.h"
#include "Interface/IAssetLibrary.h"

/// <summary>
/// 動作確認用に、カメラと光と回る立方体3個とモデルを置く。
/// </summary>
void BuildDemoScene(KujataEngine::Scene* scene, KujataEngine::IAssetLibrary* assets);
