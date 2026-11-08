#include "PipelineState.h"

#include "Core/Assert.h"

#include <cstdint>

using Microsoft::WRL::ComPtr;

namespace KujataEngine {

void PipelineState::Initialize(ID3D12Device* device, IDxcBlob* vertexShader, IDxcBlob* pixelShader, DXGI_FORMAT renderTargetFormat) {
	CreateRootSignature(device);
	CreatePipelineState(device, vertexShader, pixelShader, renderTargetFormat);
}

void PipelineState::Bind(ID3D12GraphicsCommandList* list) {
	list->SetPipelineState(pipelineState_.Get());
	list->SetGraphicsRootSignature(rootSignature_.Get());
}

void PipelineState::CreateRootSignature(ID3D12Device* device) {
	// 頂点シェーダーのb0で行列を受け取る。
	D3D12_ROOT_PARAMETER transformParameter = {};
	transformParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	transformParameter.Descriptor.ShaderRegister = 0;
	transformParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	D3D12_ROOT_SIGNATURE_DESC description = {};
	description.NumParameters = 1;
	description.pParameters = &transformParameter;
	description.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ComPtr<ID3DBlob> serialized;
	ComPtr<ID3DBlob> errors;
	HRESULT result = D3D12SerializeRootSignature(&description, D3D_ROOT_SIGNATURE_VERSION_1, &serialized, &errors);
	KUJATA_ASSERT(SUCCEEDED(result));

	result = device->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	KUJATA_ASSERT(SUCCEEDED(result));
}

void PipelineState::CreatePipelineState(ID3D12Device* device, IDxcBlob* vertexShader, IDxcBlob* pixelShader, DXGI_FORMAT renderTargetFormat) {
	// 頂点バッファの中身の並びをシェーダーのPOSITIONと結びつける。
	D3D12_INPUT_ELEMENT_DESC inputElement = {};
	inputElement.SemanticName = "POSITION";
	inputElement.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElement.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElement.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC description = {};
	description.pRootSignature = rootSignature_.Get();
	description.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
	description.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };
	description.InputLayout = { &inputElement, 1 };
	description.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	description.SampleMask = UINT32_MAX;
	description.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	description.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	description.RasterizerState.DepthClipEnable = TRUE;
	description.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	description.NumRenderTargets = 1;
	description.RTVFormats[0] = renderTargetFormat;
	description.SampleDesc.Count = 1;

	const HRESULT result = device->CreateGraphicsPipelineState(&description, IID_PPV_ARGS(&pipelineState_));
	KUJATA_ASSERT(SUCCEEDED(result));
}

} // namespace KujataEngine
