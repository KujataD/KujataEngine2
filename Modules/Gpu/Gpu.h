#pragma once

#include "CommandContext.h"
#include "Core/IModule.h"
#include "GpuDebugMessages.h"
#include "GpuDevice.h"
#include "SwapChain.h"

namespace KujataEngine {

/// <summary>
/// GPUの部品を順番に動かして、毎フレーム画面に出すモジュール。
/// </summary>
class Gpu : public IModule {
public:
	void Startup(ServiceRegistry& services, FrameLoop& frameLoop) override;
	void Shutdown() override;

private:
	void BeginFrame();
	void EndFrame();
	void StopIfGpuErrorFound();

	// メンバーは宣言の逆順に破棄されるので、使われる側を先に宣言する。
	GpuDebugMessages debugMessages_;
	GpuDevice device_;
	CommandContext commandContext_;
	SwapChain swapChain_;
};

} // namespace KujataEngine
