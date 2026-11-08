#include "Gpu.h"

#include "Core/Assert.h"

namespace KujataEngine {

// 背景を塗る色で、RGBAを0から1で表す。
static const float kClearColor[4] = { 0.392f, 0.584f, 0.929f, 1.0f };

void Gpu::Startup(ServiceRegistry& services, FrameLoop& frameLoop) {
	// Startupは追加した順に呼ばれるので、WindowはGpuより先にAddしておく。
	IWindow* window = static_cast<IWindow*>(services.Get("Window"));
	KUJATA_ASSERT(window != nullptr);

	debugMessages_.EnableDebugLayer();

	device_.Initialize();
	commandContext_.Initialize(device_.GetDevice());
	swapChain_.Initialize(device_.GetFactory(), device_.GetDevice(), commandContext_.GetQueue(), window);

	debugMessages_.AttachToDevice(device_.GetDevice());

	frameLoop.Register(kBeginFrame, [this]() {
		BeginFrame();
	});
	frameLoop.Register(kEndFrame, [this]() {
		EndFrame();
	});
}

void Gpu::Shutdown() {
	debugMessages_.PrintProblems();
}

void Gpu::BeginFrame() {
	commandContext_.Begin();

	ID3D12GraphicsCommandList* list = commandContext_.GetList();
	swapChain_.TransitionToRenderTarget(list);

	// 描き先を決めて背景を塗る。
	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView = swapChain_.GetCurrentRenderTargetView();
	list->OMSetRenderTargets(1, &renderTargetView, FALSE, nullptr);
	list->ClearRenderTargetView(renderTargetView, kClearColor, 0, nullptr);
}

void Gpu::EndFrame() {
	swapChain_.TransitionToPresent(commandContext_.GetList());
	commandContext_.Execute();

	// 命令の間違いはGPUへ渡すときに検出されるので、渡した直後に調べる。
	StopIfGpuErrorFound();

	swapChain_.Present();

	// 次のフレームで命令書を白紙に戻せるように、GPUの完了を待つ。
	commandContext_.WaitForGpu();
}

void Gpu::StopIfGpuErrorFound() {
	// Releaseではアサートが消えるので、呼び出しを条件の中に書かない。
	const bool hasError = debugMessages_.PrintProblems();
	KUJATA_ASSERT(!hasError);
}

} // namespace KujataEngine
