#include "Gpu.h"

#include <cstdio>
#include <vector>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

using Microsoft::WRL::ComPtr;

namespace KujataEngine {

// これより古い機能レベルのGPUは使わない。
static const D3D_FEATURE_LEVEL kMinFeatureLevel = D3D_FEATURE_LEVEL_12_0;

void Gpu::Startup(ServiceRegistry& services, FrameLoop& frameLoop) {
	// デバッグレイヤーはDeviceを作る前に有効にしないと効かない。
	EnableDebugLayer();

	const bool isReady = CreateFactory() && CreateDevice() && CreateCommandObjects() && CreateFence();
	if (!isReady) {
		frameLoop.RequestQuit();
		return;
	}

	SetUpDebugMessages();
}

void Gpu::Shutdown() {
	PrintDebugMessages();

	if (fenceEvent_ != nullptr) {
		CloseHandle(fenceEvent_);
		fenceEvent_ = nullptr;
	}
}

void Gpu::EnableDebugLayer() {
#ifdef _DEBUG
	ComPtr<ID3D12Debug> debugController;
	if (FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		printf("D3D12のデバッグレイヤーを使えません。\n");
		return;
	}

	debugController->EnableDebugLayer();
#endif
}

bool Gpu::CreateFactory() {
	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory_)))) {
		printf("DXGI Factoryを作れませんでした。\n");
		return false;
	}

	return true;
}

bool Gpu::CreateDevice() {
	ComPtr<IDXGIAdapter4> adapter;

	// 高性能なGPUから順に試して、最初に使えたものに決める。
	for (UINT i = 0; factory_->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND; i++) {
		DXGI_ADAPTER_DESC3 description = {};
		adapter->GetDesc3(&description);

		// CPUで動くGPUの代わりは遅いので使わない。
		if (description.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE) {
			continue;
		}

		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), kMinFeatureLevel, IID_PPV_ARGS(&device_)))) {
			printf("GPU: %ls\n", description.Description);
			return true;
		}
	}

	printf("使えるGPUが見つかりませんでした。\n");
	return false;
}

bool Gpu::CreateCommandObjects() {
	D3D12_COMMAND_QUEUE_DESC queueDescription = {};
	if (FAILED(device_->CreateCommandQueue(&queueDescription, IID_PPV_ARGS(&commandQueue_)))) {
		printf("CommandQueueを作れませんでした。\n");
		return false;
	}

	if (FAILED(device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_)))) {
		printf("CommandAllocatorを作れませんでした。\n");
		return false;
	}

	if (FAILED(device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_.Get(), nullptr, IID_PPV_ARGS(&commandList_)))) {
		printf("CommandListを作れませんでした。\n");
		return false;
	}

	// 作った直後は書き込み中の状態なので、毎フレームResetできるように閉じておく。
	commandList_->Close();
	return true;
}

bool Gpu::CreateFence() {
	if (FAILED(device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_)))) {
		printf("Fenceを作れませんでした。\n");
		return false;
	}

	fenceEvent_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (fenceEvent_ == nullptr) {
		printf("Fenceのイベントを作れませんでした。\n");
		return false;
	}

	return true;
}

void Gpu::SetUpDebugMessages() {
#ifdef _DEBUG
	// デバッグレイヤーが入っていない環境では取得できない。
	if (FAILED(device_.As(&infoQueue_))) {
		return;
	}

	// デバッガーなしで止めると異常終了するので、付いているときだけ止める。
	if (IsDebuggerPresent()) {
		infoQueue_->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		infoQueue_->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
	}
#endif
}

void Gpu::PrintDebugMessages() {
	if (infoQueue_ == nullptr) {
		return;
	}

	const UINT64 messageCount = infoQueue_->GetNumStoredMessages();
	for (UINT64 i = 0; i < messageCount; i++) {
		// 1回目で必要な大きさを聞いて、2回目で中身を受け取る。
		SIZE_T messageSize = 0;
		infoQueue_->GetMessage(i, nullptr, &messageSize);

		std::vector<char> buffer(messageSize);
		D3D12_MESSAGE* message = reinterpret_cast<D3D12_MESSAGE*>(buffer.data());
		infoQueue_->GetMessage(i, message, &messageSize);

		// 情報だけのメッセージは数が多いので、問題があるものだけを出す。
		const bool isProblem = message->Severity != D3D12_MESSAGE_SEVERITY_INFO && message->Severity != D3D12_MESSAGE_SEVERITY_MESSAGE;
		if (isProblem) {
			printf("[D3D12] %s\n", message->pDescription);
		}
	}

	infoQueue_->ClearStoredMessages();
}

} // namespace KujataEngine
