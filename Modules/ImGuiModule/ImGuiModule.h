#pragma once

#include "Core/IModule.h"
#include "Interface/IGpu.h"
#include "Interface/IWindow.h"

#include <d3d12.h>
#include <vector>
#include <wrl/client.h>

namespace KujataEngine {

/// <summary>
/// imguiを初期化し、毎フレームの開始と描画をするモジュールで、USE_IMGUIがない構成では何もしない。
/// </summary>
class ImGuiModule : public IModule {
public:
	std::vector<std::string> GetRequiredServices() override;
	void Startup(ServiceRegistry& services, FrameLoop& frameLoop) override;
	void Shutdown() override;

	/// <summary>
	/// imguiがテクスチャを置く場所を、棚から1つ確保して返す。
	/// </summary>
	UINT AllocateDescriptor();

	/// <summary>
	/// imguiが使い終えた場所を、次の確保で使い回せるように戻す。
	/// </summary>
	void FreeDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE handle);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(UINT index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(UINT index);

private:
	void CreateDescriptorHeap();
	void BeginFrame();
	void Draw();

	IGpu* gpu_ = nullptr;

	// imguiのフォントなどのテクスチャの名札を置く、このモジュールだけの棚。
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap_;
	UINT descriptorSize_ = 0;
	UINT allocatedCount_ = 0;

	// 戻された場所の番号で、新しい場所より先に使う。
	std::vector<UINT> freeIndices_;
};

} // namespace KujataEngine
