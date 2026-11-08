#include "Gpu.h"

#include "Core/Assert.h"

namespace KujataEngine {

// 背景を塗る色で、RGBAを0から1で表す。
static const float kClearColor[4] = { 0.392f, 0.584f, 0.929f, 1.0f };

std::vector<std::string> Gpu::GetProvidedServices() {
	return { kGpuServiceName };
}

std::vector<std::string> Gpu::GetRequiredServices() {
	return { kWindowServiceName };
}

void Gpu::Startup(ServiceRegistry& services, FrameLoop& frameLoop) {
	IWindow* window = static_cast<IWindow*>(services.Get(kWindowServiceName));
	KUJATA_ASSERT(window != nullptr);

	debugMessages_.EnableDebugLayer();

	device_.Initialize();
	commandContext_.Initialize(device_.GetDevice());
	swapChain_.Initialize(device_.GetFactory(), device_.GetDevice(), commandContext_.GetQueue(), window);
	depthBuffer_.Initialize(device_.GetDevice(), window->GetClientWidth(), window->GetClientHeight());

	debugMessages_.AttachToDevice(device_.GetDevice());

	// 受け取る側がIGpu*に戻せるように、void*にする前にIGpu*へ変換する。
	services.Register(kGpuServiceName, static_cast<IGpu*>(this));

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

ID3D12Device* Gpu::GetDevice() {
	return device_.GetDevice();
}

ID3D12GraphicsCommandList* Gpu::GetCommandList() {
	return commandContext_.GetList();
}

DXGI_FORMAT Gpu::GetRenderTargetFormat() {
	return swapChain_.GetRenderTargetFormat();
}

DXGI_FORMAT Gpu::GetDepthFormat() {
	return depthBuffer_.GetFormat();
}

void Gpu::ExecuteImmediately(std::function<void(ID3D12GraphicsCommandList*)> recordCommands) {
	commandContext_.Begin();
	recordCommands(commandContext_.GetList());
	commandContext_.Execute();
	commandContext_.WaitForGpu();
}

void Gpu::BeginFrame() {
	commandContext_.Begin();

	ID3D12GraphicsCommandList* list = commandContext_.GetList();
	swapChain_.TransitionToRenderTarget(list);

	// 描き先を決めて背景と深度を初期化する。
	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView = swapChain_.GetCurrentRenderTargetView();
	D3D12_CPU_DESCRIPTOR_HANDLE depthView = depthBuffer_.GetView();
	list->OMSetRenderTargets(1, &renderTargetView, FALSE, &depthView);
	list->ClearRenderTargetView(renderTargetView, kClearColor, 0, nullptr);
	depthBuffer_.Clear(list);
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
