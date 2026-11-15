#include "ImGuiModule.h"

#include "Core/Assert.h"

#ifdef USE_IMGUI
#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>

// imguiのヘッダーでは宣言がコメントになっているので、使う側で宣言する。
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif // USE_IMGUI

namespace KujataEngine {

#ifdef USE_IMGUI

// imguiのテクスチャの名札を置ける数。
static const UINT kDescriptorHeapCapacity = 64;

// imguiが頂点のバッファを使い回す枚数で、画面に出す画像の枚数とそろえる。
static const int kFramesInFlight = 2;

/// <summary>
/// imguiから名札の場所を頼まれたときに呼ばれ、モジュールの棚から確保して渡す。
/// </summary>
static void AllocateImGuiDescriptor(ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* outCpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE* outGpuHandle) {
	ImGuiModule* module = static_cast<ImGuiModule*>(info->UserData);
	const UINT index = module->AllocateDescriptor();
	*outCpuHandle = module->GetCpuHandle(index);
	*outGpuHandle = module->GetGpuHandle(index);
}

/// <summary>
/// imguiが名札の場所を使い終えたときに呼ばれ、モジュールの棚へ戻す。
/// </summary>
static void FreeImGuiDescriptor(ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE) {
	ImGuiModule* module = static_cast<ImGuiModule*>(info->UserData);
	module->FreeDescriptor(cpuHandle);
}

#endif // USE_IMGUI

std::vector<std::string> ImGuiModule::GetRequiredServices() {
	return { kGpuServiceName, kWindowServiceName };
}

void ImGuiModule::Startup(ServiceRegistry& services, FrameLoop& frameLoop) {
#ifdef USE_IMGUI
	gpu_ = static_cast<IGpu*>(services.Get(kGpuServiceName));
	KUJATA_ASSERT(gpu_ != nullptr);

	IWindow* window = static_cast<IWindow*>(services.Get(kWindowServiceName));
	KUJATA_ASSERT(window != nullptr);

	CreateDescriptorHeap();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// ウィンドウどうしをつなげて並べられるようにする。
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui_ImplWin32_Init(window->GetNativeHandle());

	ImGui_ImplDX12_InitInfo initInfo = {};
	initInfo.Device = gpu_->GetDevice();
	initInfo.CommandQueue = gpu_->GetCommandQueue();
	initInfo.NumFramesInFlight = kFramesInFlight;
	initInfo.RTVFormat = gpu_->GetRenderTargetFormat();
	initInfo.DSVFormat = gpu_->GetDepthFormat();
	initInfo.SrvDescriptorHeap = descriptorHeap_.Get();
	initInfo.UserData = this;
	initInfo.SrvDescriptorAllocFn = AllocateImGuiDescriptor;
	initInfo.SrvDescriptorFreeFn = FreeImGuiDescriptor;
	const bool isRendererReady = ImGui_ImplDX12_Init(&initInfo);
	KUJATA_ASSERT(isRendererReady);

	// マウスやキーのメッセージを、窓より先にimguiへ渡す。
	window->AddMessageHandler([](void* windowHandle, uint32_t message, uint64_t wparam, int64_t lparam) {
		return ImGui_ImplWin32_WndProcHandler(static_cast<HWND>(windowHandle), message, static_cast<WPARAM>(wparam), static_cast<LPARAM>(lparam)) != 0;
	});

	frameLoop.Register(kBeginFrame, [this]() {
		BeginFrame();
	});
	// モジュールの起動の順に関係なく、シーンの上に重なるようにする。
	frameLoop.Register(kRenderOverlay, [this]() {
		Draw();
	});
#endif // USE_IMGUI
}

void ImGuiModule::Shutdown() {
#ifdef USE_IMGUI
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif // USE_IMGUI
}

UINT ImGuiModule::AllocateDescriptor() {
	// 戻された場所があれば、先に使う。
	if (!freeIndices_.empty()) {
		const UINT index = freeIndices_.back();
		freeIndices_.pop_back();
		return index;
	}

	KUJATA_ASSERT(allocatedCount_ < descriptorHeap_->GetDesc().NumDescriptors);
	const UINT index = allocatedCount_;
	allocatedCount_++;
	return index;
}

void ImGuiModule::FreeDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE handle) {
	// 棚の先頭からの距離を名札1つの大きさで割ると、番号に戻る。
	const SIZE_T heapStart = descriptorHeap_->GetCPUDescriptorHandleForHeapStart().ptr;
	freeIndices_.push_back(static_cast<UINT>((handle.ptr - heapStart) / descriptorSize_));
}

D3D12_CPU_DESCRIPTOR_HANDLE ImGuiModule::GetCpuHandle(UINT index) {
	D3D12_CPU_DESCRIPTOR_HANDLE handle = descriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += static_cast<SIZE_T>(descriptorSize_) * index;
	return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE ImGuiModule::GetGpuHandle(UINT index) {
	D3D12_GPU_DESCRIPTOR_HANDLE handle = descriptorHeap_->GetGPUDescriptorHandleForHeapStart();
	handle.ptr += static_cast<UINT64>(descriptorSize_) * index;
	return handle;
}

#ifdef USE_IMGUI

void ImGuiModule::CreateDescriptorHeap() {
	ID3D12Device* device = gpu_->GetDevice();

	D3D12_DESCRIPTOR_HEAP_DESC description = {};
	description.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	description.NumDescriptors = kDescriptorHeapCapacity;
	description.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	const HRESULT result = device->CreateDescriptorHeap(&description, IID_PPV_ARGS(&descriptorHeap_));
	KUJATA_ASSERT(SUCCEEDED(result));

	descriptorSize_ = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void ImGuiModule::BeginFrame() {
	// ここからDrawまでの間に、ゲームやほかのモジュールがImGuiの関数でウィンドウを組む。
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void ImGuiModule::Draw() {
	ImGui::Render();

	// Rendererの棚に切り替わっているので、imguiの棚に戻してから描く。
	ID3D12GraphicsCommandList* list = gpu_->GetCommandList();
	ID3D12DescriptorHeap* heaps[] = { descriptorHeap_.Get() };
	list->SetDescriptorHeaps(1, heaps);
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), list);
}

#endif // USE_IMGUI

} // namespace KujataEngine
