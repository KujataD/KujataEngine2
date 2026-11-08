#include "ShaderCompiler.h"

#include "Core/Assert.h"

#include <cstdio>
#include <vector>

#pragma comment(lib, "dxcompiler.lib")

using Microsoft::WRL::ComPtr;

namespace KujataEngine {

void ShaderCompiler::Initialize() {
	HRESULT result = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils_));
	KUJATA_ASSERT(SUCCEEDED(result));

	result = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler_));
	KUJATA_ASSERT(SUCCEEDED(result));

	result = utils_->CreateDefaultIncludeHandler(&includeHandler_);
	KUJATA_ASSERT(SUCCEEDED(result));
}

ComPtr<IDxcBlob> ShaderCompiler::Compile(const std::wstring& filePath, const wchar_t* profile) {
	// ファイルがないのは書いた人の間違いなので、止めずにログへ出す。
	UINT32 codePage = DXC_CP_UTF8;
	ComPtr<IDxcBlobEncoding> sourceFile;
	if (FAILED(utils_->LoadFile(filePath.c_str(), &codePage, &sourceFile))) {
		printf("シェーダーのファイルを読めませんでした: %ls\n", filePath.c_str());
		return nullptr;
	}

	DxcBuffer source = {};
	source.Ptr = sourceFile->GetBufferPointer();
	source.Size = sourceFile->GetBufferSize();
	source.Encoding = DXC_CP_UTF8;

	// -Zprは、行列をC++と同じ行優先のメモリの並びで扱う指定。
	std::vector<LPCWSTR> arguments = { filePath.c_str(), L"-E", L"main", L"-T", profile, L"-Zpr" };
#ifdef _DEBUG
	arguments.push_back(L"-Zi");
	arguments.push_back(L"-Qembed_debug");
	arguments.push_back(L"-Od");
#endif

	ComPtr<IDxcResult> compileResult;
	HRESULT result = compiler_->Compile(&source, arguments.data(), static_cast<UINT32>(arguments.size()), includeHandler_.Get(), IID_PPV_ARGS(&compileResult));
	KUJATA_ASSERT(SUCCEEDED(result));

	ComPtr<IDxcBlobUtf8> errors;
	result = compileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
	KUJATA_ASSERT(SUCCEEDED(result));
	if (errors != nullptr && errors->GetStringLength() > 0) {
		printf("[シェーダー] %s\n", errors->GetStringPointer());
	}

	HRESULT compileStatus = S_OK;
	result = compileResult->GetStatus(&compileStatus);
	KUJATA_ASSERT(SUCCEEDED(result));
	if (FAILED(compileStatus)) {
		return nullptr;
	}

	ComPtr<IDxcBlob> shader;
	result = compileResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shader), nullptr);
	KUJATA_ASSERT(SUCCEEDED(result));
	return shader;
}

} // namespace KujataEngine
