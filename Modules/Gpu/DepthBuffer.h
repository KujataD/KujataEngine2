#pragma once

#include <d3d12.h>
#include <wrl/client.h>

namespace KujataEngine {

/// <summary>
/// ピクセルごとの奥行きを覚えて、手前の物だけを残すためのバッファ。
/// </summary>
class DepthBuffer {
public:
	/// <summary>
	/// 指定の大きさで深度バッファと名札を作る。
	/// </summary>
	void Initialize(ID3D12Device* device, UINT width, UINT height);

	/// <summary>
	/// 深度バッファに描くための名札を返す。
	/// </summary>
	D3D12_CPU_DESCRIPTOR_HANDLE GetView();

	/// <summary>
	/// 深度の形式を返す。
	/// </summary>
	DXGI_FORMAT GetFormat();

	/// <summary>
	/// 深度をいちばん遠い値に戻す命令を書く。
	/// </summary>
	void Clear(ID3D12GraphicsCommandList* list);

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> buffer_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> viewHeap_;
};

} // namespace KujataEngine
