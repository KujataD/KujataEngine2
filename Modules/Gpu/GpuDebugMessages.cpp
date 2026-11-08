#include "GpuDebugMessages.h"

#include "Core/Assert.h"

#include <cstdio>
#include <vector>

using Microsoft::WRL::ComPtr;

namespace KujataEngine {

void GpuDebugMessages::EnableDebugLayer() {
#ifdef _DEBUG
	ComPtr<ID3D12Debug> debugController;
	if (FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		printf("D3D12のデバッグレイヤーを使えません。\n");
		return;
	}

	debugController->EnableDebugLayer();
#endif
}

void GpuDebugMessages::AttachToDevice(ID3D12Device* device) {
#ifdef _DEBUG
	// デバッグレイヤーが入っていない環境ではnullptrのままになる。
	device->QueryInterface(IID_PPV_ARGS(&infoQueue_));
#endif
}

bool GpuDebugMessages::PrintProblems() {
	if (infoQueue_ == nullptr) {
		return false;
	}

	bool hasError = false;

	const UINT64 messageCount = infoQueue_->GetNumStoredMessages();
	for (UINT64 i = 0; i < messageCount; i++) {
		// 1回目で必要な大きさを聞いて、2回目で中身を受け取る。
		SIZE_T messageSize = 0;
		HRESULT result = infoQueue_->GetMessage(i, nullptr, &messageSize);
		KUJATA_ASSERT(SUCCEEDED(result));

		std::vector<char> buffer(messageSize);
		D3D12_MESSAGE* message = reinterpret_cast<D3D12_MESSAGE*>(buffer.data());
		result = infoQueue_->GetMessage(i, message, &messageSize);
		KUJATA_ASSERT(SUCCEEDED(result));

		// 情報だけのメッセージは数が多いので、問題があるものだけを出す。
		const bool isProblem = message->Severity != D3D12_MESSAGE_SEVERITY_INFO && message->Severity != D3D12_MESSAGE_SEVERITY_MESSAGE;
		if (isProblem) {
			printf("[D3D12] %s\n", message->pDescription);
		}

		if (message->Severity == D3D12_MESSAGE_SEVERITY_CORRUPTION || message->Severity == D3D12_MESSAGE_SEVERITY_ERROR) {
			hasError = true;
		}
	}

	infoQueue_->ClearStoredMessages();
	return hasError;
}

} // namespace KujataEngine
