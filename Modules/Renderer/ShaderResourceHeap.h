#pragma once

#include <d3d12.h>
#include <wrl/client.h>

namespace KujataEngine {

/// <summary>
/// テクスチャの名札を並べておく、シェーダーから見える棚。
/// </summary>
class ShaderResourceHeap {
public:
	/// <summary>
	/// 指定の数だけ名札を置ける棚を作る。
	/// </summary>
	void Initialize(ID3D12Device* device, UINT capacity);

	/// <summary>
	/// 空いている場所を1つ確保して、その番号を返す。
	/// </summary>
	UINT Allocate();

	/// <summary>
	/// まだ名札を置ける場所があればtrueを返す。
	/// </summary>
	bool HasFreeSlot();

	D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(UINT index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(UINT index);

	/// <summary>
	/// この棚を使う設定を命令書に書く。
	/// </summary>
	void Bind(ID3D12GraphicsCommandList* list);

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap_;
	UINT capacity_ = 0;
	UINT allocatedCount_ = 0;
	UINT descriptorSize_ = 0;
};

} // namespace KujataEngine
