#pragma once

#include <d3d12.h>
#include <wrl/client.h>

namespace KujataEngine {

/// <summary>
/// 入口の一覧の説明からRootSignatureを作る。
/// </summary>
Microsoft::WRL::ComPtr<ID3D12RootSignature> CreateRootSignatureFromDescription(ID3D12Device* device, const D3D12_ROOT_SIGNATURE_DESC& description);

/// <summary>
/// 名札を1つだけ受け取るテーブルの入口を作る。
/// </summary>
D3D12_ROOT_PARAMETER MakeTableParameter(const D3D12_DESCRIPTOR_RANGE* range, D3D12_SHADER_VISIBILITY visibility);

/// <summary>
/// 指定の種類とレジスタの番号の名札1つ分の範囲を作る。
/// </summary>
D3D12_DESCRIPTOR_RANGE MakeSingleRange(D3D12_DESCRIPTOR_RANGE_TYPE type, UINT shaderRegister);

} // namespace KujataEngine
