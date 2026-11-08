#pragma once

#include <d3d12.h>
#include <wrl/client.h>

namespace KujataEngine {

/// <summary>
/// シェーダーへ毎フレーム渡す値を、描く物の数だけ並べて置くバッファ。
/// </summary>
class ConstantBuffer {
public:
	/// <summary>
	/// 指定の大きさの値を指定の数だけ置けるバッファを作る。
	/// </summary>
	void Initialize(ID3D12Device* device, UINT dataSize, UINT capacity);

	/// <summary>
	/// 指定の番号の場所へ値を書き込む。
	/// </summary>
	void Write(UINT index, const void* data);

	/// <summary>
	/// 指定の番号の値を使う設定を命令書に書く。
	/// </summary>
	void Bind(ID3D12GraphicsCommandList* list, UINT rootParameterIndex, UINT index);

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> buffer_;
	unsigned char* mappedMemory_ = nullptr;
	UINT dataSize_ = 0;
	UINT alignedSize_ = 0;
	UINT capacity_ = 0;
};

} // namespace KujataEngine
