#pragma once

#include "Core/IModule.h"

namespace KujataEngine {

/// <summary>
/// Audioの機能をAppに組み込むモジュール。
/// </summary>
class Audio : public IModule {
public:
	void Startup(ServiceRegistry& services, FrameLoop& frameLoop) override;
};

} // namespace KujataEngine
