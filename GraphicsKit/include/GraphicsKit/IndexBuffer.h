#pragma once

#include <cstdint>
#include <d3d12.h>
#include <wrl/client.h>

namespace KujataEngine {

/// <summary>
/// 頂点を使う順番をGPUから読める場所に置くクラス。
/// </summary>
class IndexBuffer {
public:
	/// <summary>
	/// バッファを作って順番のデータを書き込む。
	/// </summary>
	void Initialize(ID3D12Device* device, const uint32_t* indices, UINT indexCount);

	/// <summary>
	/// この順番を使う設定を命令書に書く。
	/// </summary>
	void Bind(ID3D12GraphicsCommandList* list);

	UINT GetIndexCount();

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> buffer_;
	D3D12_INDEX_BUFFER_VIEW view_ = {};
	UINT indexCount_ = 0;
};

} // namespace KujataEngine
