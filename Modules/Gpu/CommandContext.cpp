#include "CommandContext.h"

#include "Core/Assert.h"

namespace KujataEngine {

CommandContext::~CommandContext() {
	if (fenceEvent_ != nullptr) {
		CloseHandle(fenceEvent_);
	}
}

void CommandContext::Initialize(ID3D12Device* device) {
	CreateCommandObjects(device);
	CreateFence(device);
}

void CommandContext::Begin() {
	HRESULT result = allocator_->Reset();
	KUJATA_ASSERT(SUCCEEDED(result));

	result = list_->Reset(allocator_.Get(), nullptr);
	KUJATA_ASSERT(SUCCEEDED(result));
}

void CommandContext::Execute() {
	const HRESULT result = list_->Close();
	KUJATA_ASSERT(SUCCEEDED(result));

	ID3D12CommandList* lists[] = { list_.Get() };
	queue_->ExecuteCommandLists(1, lists);
}

void CommandContext::WaitForGpu() {
	// 数字を増やさないと、前のフレームの完了で待ちが終わってしまう。
	fenceValue_++;
	HRESULT result = queue_->Signal(fence_.Get(), fenceValue_);
	KUJATA_ASSERT(SUCCEEDED(result));

	if (fence_->GetCompletedValue() < fenceValue_) {
		result = fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
		KUJATA_ASSERT(SUCCEEDED(result));

		const DWORD waitResult = WaitForSingleObject(fenceEvent_, INFINITE);
		KUJATA_ASSERT(waitResult == WAIT_OBJECT_0);
	}
}

ID3D12CommandQueue* CommandContext::GetQueue() {
	return queue_.Get();
}

ID3D12GraphicsCommandList* CommandContext::GetList() {
	return list_.Get();
}

void CommandContext::CreateCommandObjects(ID3D12Device* device) {
	D3D12_COMMAND_QUEUE_DESC queueDescription = {};
	HRESULT result = device->CreateCommandQueue(&queueDescription, IID_PPV_ARGS(&queue_));
	KUJATA_ASSERT(SUCCEEDED(result));

	result = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator_));
	KUJATA_ASSERT(SUCCEEDED(result));

	result = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator_.Get(), nullptr, IID_PPV_ARGS(&list_));
	KUJATA_ASSERT(SUCCEEDED(result));

	// 作った直後は書き込み中の状態なので、毎フレームResetできるように閉じておく。
	result = list_->Close();
	KUJATA_ASSERT(SUCCEEDED(result));
}

void CommandContext::CreateFence(ID3D12Device* device) {
	const HRESULT result = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
	KUJATA_ASSERT(SUCCEEDED(result));

	fenceEvent_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	KUJATA_ASSERT(fenceEvent_ != nullptr);
}

} // namespace KujataEngine
