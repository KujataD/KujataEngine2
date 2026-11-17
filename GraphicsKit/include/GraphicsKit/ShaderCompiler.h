#pragma once

#include <Windows.h>
#include <d3d12shader.h>
#include <dxcapi.h>
#include <string>
#include <wrl/client.h>

namespace KujataEngine {

/// <summary>
/// HLSLのファイルをGPUで動く形式に変換するクラス。
/// </summary>
class ShaderCompiler {
public:
	/// <summary>
	/// シェーダーのコンパイラーを準備する。
	/// </summary>
	void Initialize();

	/// <summary>
	/// HLSLをコンパイルし、書き間違いがあれば内容をログに出してnullptrを返す。
	/// </summary>
	Microsoft::WRL::ComPtr<IDxcBlob> Compile(const std::wstring& filePath, const wchar_t* profile);

private:
	Microsoft::WRL::ComPtr<IDxcUtils> utils_;
	Microsoft::WRL::ComPtr<IDxcCompiler3> compiler_;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;
};

} // namespace KujataEngine
