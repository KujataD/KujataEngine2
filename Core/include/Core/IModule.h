#pragma once

#include "Core/FrameLoop.h"
#include "Core/ServiceRegistry.h"

/// <summary>
/// Appに組み込める機能のまとまりで、各モジュールはこれを継承する。
/// </summary>
class IModule {
public:
	virtual ~IModule() = default;

	/// <summary>
	/// 起動時に1回呼ばれ、サービスとフレームの段階への登録をする。
	/// </summary>
	virtual void Startup(ServiceRegistry& services, FrameLoop& frameLoop) = 0;

	/// <summary>
	/// 終了時に1回呼ばれる。
	/// </summary>
	virtual void Shutdown() {}
};
