#pragma once

#include <d3d12.h>
#include <wrl/client.h>

namespace KujataEngine {

// 入口の決まり：定数は定数バッファの場所をルートCBVで直接渡し、バッファとテクスチャは棚に置いた名札を表で渡す。

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

/// <summary>
/// 定数バッファの場所を直接受け取る入口を作る。
/// </summary>
D3D12_ROOT_PARAMETER MakeConstantsParameter(UINT shaderRegister, D3D12_SHADER_VISIBILITY visibility);

/// <summary>
/// ミップの間も補間する線形で読み、0から1の外は繰り返すサンプラーを作る。
/// </summary>
D3D12_STATIC_SAMPLER_DESC MakeWrapLinearSampler(UINT shaderRegister, D3D12_SHADER_VISIBILITY visibility);

} // namespace KujataEngine
