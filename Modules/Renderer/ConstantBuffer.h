#pragma once

#include <d3d12.h>
#include <wrl/client.h>

namespace KujataEngine {

/// <summary>
/// シェーダーへ毎フレーム渡す値を置くバッファ。
/// </summary>
class ConstantBuffer {
public:
	/// <summary>
	/// 指定の大きさのバッファを作って書き込める状態にする。
	/// </summary>
	void Initialize(ID3D12Device* device, UINT dataSize);

	/// <summary>
	/// 値を書き込み、作るときに指定した大きさ分をコピーする。
	/// </summary>
	void Write(const void* data);

	/// <summary>
	/// このバッファを使う設定を命令書に書く。
	/// </summary>
	void Bind(ID3D12GraphicsCommandList* list, UINT rootParameterIndex);

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> buffer_;
	void* mappedMemory_ = nullptr;
	UINT dataSize_ = 0;
};

} // namespace KujataEngine
