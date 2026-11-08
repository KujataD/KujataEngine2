#pragma once

#include "Core/IModule.h"

/// <summary>
/// Audioの機能をAppに組み込むモジュール。
/// </summary>
class Audio : public IModule {
public:
	void Startup(ServiceRegistry& services, FrameLoop& frameLoop) override;
};
