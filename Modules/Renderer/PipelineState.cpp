#include "PipelineState.h"

#include "Core/Assert.h"

#include <cstdint>

using Microsoft::WRL::ComPtr;

namespace KujataEngine {

void PipelineState::Initialize(ID3D12Device* device, IDxcBlob* vertexShader, IDxcBlob* pixelShader, DXGI_FORMAT renderTargetFormat, DXGI_FORMAT depthFormat) {
	CreateRootSignature(device);
	CreatePipelineState(device, vertexShader, pixelShader, renderTargetFormat, depthFormat);
}

void PipelineState::Bind(ID3D12GraphicsCommandList* list) {
	list->SetPipelineState(pipelineState_.Get());
	list->SetGraphicsRootSignature(rootSignature_.Get());
}

void PipelineState::CreateRootSignature(ID3D12Device* device) {
	D3D12_DESCRIPTOR_RANGE textureRange = {};
	textureRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	textureRange.NumDescriptors = 1;
	textureRange.BaseShaderRegister = 0;
	textureRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameters[2] = {};

	// 頂点シェーダーのb0で行列を受け取る。
	rootParameters[kTransformRootParameterIndex].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[kTransformRootParameterIndex].Descriptor.ShaderRegister = 0;
	rootParameters[kTransformRootParameterIndex].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	// ピクセルシェーダーのt0でテクスチャを受け取る。
	rootParameters[kTextureRootParameterIndex].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[kTextureRootParameterIndex].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[kTextureRootParameterIndex].DescriptorTable.pDescriptorRanges = &textureRange;
	rootParameters[kTextureRootParameterIndex].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// s0はバイリニアで読み、0から1の外は繰り返す。
	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_DESC description = {};
	description.NumParameters = 2;
	description.pParameters = rootParameters;
	description.NumStaticSamplers = 1;
	description.pStaticSamplers = &sampler;
	description.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ComPtr<ID3DBlob> serialized;
	ComPtr<ID3DBlob> errors;
	HRESULT result = D3D12SerializeRootSignature(&description, D3D_ROOT_SIGNATURE_VERSION_1, &serialized, &errors);
	KUJATA_ASSERT(SUCCEEDED(result));

	result = device->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	KUJATA_ASSERT(SUCCEEDED(result));
}

void PipelineState::CreatePipelineState(ID3D12Device* device, IDxcBlob* vertexShader, IDxcBlob* pixelShader, DXGI_FORMAT renderTargetFormat, DXGI_FORMAT depthFormat) {
	// 頂点バッファの中身の並びをシェーダーのPOSITIONとTEXCOORDに結びつける。
	D3D12_INPUT_ELEMENT_DESC inputElements[2] = {};
	inputElements[0].SemanticName = "POSITION";
	inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElements[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElements[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	inputElements[1].SemanticName = "TEXCOORD";
	inputElements[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElements[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElements[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC description = {};
	description.pRootSignature = rootSignature_.Get();
	description.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
	description.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };
	description.InputLayout = { inputElements, 2 };
	description.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	description.SampleMask = UINT32_MAX;
	description.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	description.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	description.RasterizerState.DepthClipEnable = TRUE;
	description.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	description.NumRenderTargets = 1;
	description.RTVFormats[0] = renderTargetFormat;
	description.DepthStencilState.DepthEnable = TRUE;
	description.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	description.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	description.DSVFormat = depthFormat;
	description.SampleDesc.Count = 1;

	const HRESULT result = device->CreateGraphicsPipelineState(&description, IID_PPV_ARGS(&pipelineState_));
	KUJATA_ASSERT(SUCCEEDED(result));
}

} // namespace KujataEngine
