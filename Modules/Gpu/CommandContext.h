#pragma once

#include <d3d12.h>
#include <wrl/client.h>

namespace KujataEngine {

/// <summary>
/// GPUへの命令書を書いて渡し、完了を待つクラス。
/// </summary>
class CommandContext {
public:
	~CommandContext();

	/// <summary>
	/// Queueと命令書とFenceを作る。
	/// </summary>
	void Initialize(ID3D12Device* device);

	/// <summary>
	/// 命令書を白紙に戻して、書き始められる状態にする。
	/// </summary>
	void Begin();

	/// <summary>
	/// 命令書を閉じてGPUへ渡す。
	/// </summary>
	void Execute();

	/// <summary>
	/// ここまでに渡した命令をGPUが終えるまで待つ。
	/// </summary>
	void WaitForGpu();

	ID3D12CommandQueue* GetQueue();
	ID3D12GraphicsCommandList* GetList();

private:
	void CreateCommandObjects(ID3D12Device* device);
	void CreateFence(ID3D12Device* device);

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue_;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator_;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> list_;

	Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
	HANDLE fenceEvent_ = nullptr;
	UINT64 fenceValue_ = 0;
};

} // namespace KujataEngine
