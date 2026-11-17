#include "GraphicsKit/RootSignature.h"

#include "Core/Assert.h"

using Microsoft::WRL::ComPtr;

namespace KujataEngine {

ComPtr<ID3D12RootSignature> CreateRootSignatureFromDescription(ID3D12Device* device, const D3D12_ROOT_SIGNATURE_DESC& description) {
	ComPtr<ID3DBlob> serialized;
	ComPtr<ID3DBlob> errors;
	HRESULT result = D3D12SerializeRootSignature(&description, D3D_ROOT_SIGNATURE_VERSION_1, &serialized, &errors);
	KUJATA_ASSERT(SUCCEEDED(result));

	ComPtr<ID3D12RootSignature> rootSignature;
	result = device->CreateRootSignature(0, serialized->GetBufferPointer(), serialized->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	KUJATA_ASSERT(SUCCEEDED(result));
	return rootSignature;
}

D3D12_ROOT_PARAMETER MakeTableParameter(const D3D12_DESCRIPTOR_RANGE* range, D3D12_SHADER_VISIBILITY visibility) {
	D3D12_ROOT_PARAMETER parameter = {};
	parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	parameter.DescriptorTable.NumDescriptorRanges = 1;
	parameter.DescriptorTable.pDescriptorRanges = range;
	parameter.ShaderVisibility = visibility;
	return parameter;
}

D3D12_DESCRIPTOR_RANGE MakeSingleRange(D3D12_DESCRIPTOR_RANGE_TYPE type, UINT shaderRegister) {
	D3D12_DESCRIPTOR_RANGE range = {};
	range.RangeType = type;
	range.NumDescriptors = 1;
	range.BaseShaderRegister = shaderRegister;
	range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	return range;
}

} // namespace KujataEngine
