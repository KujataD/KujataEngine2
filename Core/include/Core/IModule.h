#pragma once

#include "Core/FrameLoop.h"
#include "Core/ServiceRegistry.h"

#include <string>
#include <vector>

namespace KujataEngine {

/// <summary>
/// Appに組み込める機能のまとまりで、各モジュールはこれを継承する。
/// </summary>
class IModule {
public:
	virtual ~IModule() = default;

	/// <summary>
	/// Startupで登録するサービスの名前を返す。
	/// </summary>
	virtual std::vector<std::string> GetProvidedServices() {
		return {};
	}

	/// <summary>
	/// Startupで受け取るサービスの名前を返し、Appはこれを見て起動の順番を決める。
	/// </summary>
	virtual std::vector<std::string> GetRequiredServices() {
		return {};
	}

	/// <summary>
	/// 起動時に1回呼ばれ、サービスとフレームの段階への登録をする。
	/// </summary>
	virtual void Startup(ServiceRegistry& services, FrameLoop& frameLoop) = 0;

	/// <summary>
	/// 終了時に1回呼ばれる。
	/// </summary>
	virtual void Shutdown() {}
};

} // namespace KujataEngine
