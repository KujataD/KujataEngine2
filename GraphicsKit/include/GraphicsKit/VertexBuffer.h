#pragma once

#include <d3d12.h>
#include <wrl/client.h>

namespace KujataEngine {

/// <summary>
/// 頂点のデータをGPUから読める場所に置くクラス。
/// </summary>
class VertexBuffer {
public:
	/// <summary>
	/// バッファを作って頂点のデータを書き込む。
	/// </summary>
	void Initialize(ID3D12Device* device, const void* vertices, UINT vertexSize, UINT vertexCount);

	/// <summary>
	/// この頂点を使う設定を命令書に書く。
	/// </summary>
	void Bind(ID3D12GraphicsCommandList* list);

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> buffer_;
	D3D12_VERTEX_BUFFER_VIEW view_ = {};
};

} // namespace KujataEngine
