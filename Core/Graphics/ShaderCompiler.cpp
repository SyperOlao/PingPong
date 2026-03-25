#include "Core/Graphics/ShaderCompiler.h"

#include "Core/Assets/AssetPathResolver.h"

#include <d3dcompiler.h>

#include <stdexcept>
#include <string>

Microsoft::WRL::ComPtr<ID3DBlob> ShaderCompiler::CompileFromFile(
    const std::filesystem::path &filePath,
    const std::string_view entryPoint,
    const std::string_view profile
)
{
    const std::filesystem::path resolvedPath = AssetPathResolver::Resolve(filePath);

    const std::string entryPointString(entryPoint);
    const std::string profileString(profile);

    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

    const HRESULT hr = D3DCompileFromFile(
        resolvedPath.c_str(),
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPointString.c_str(),
        profileString.c_str(),
        flags,
        0,
        shaderBlob.GetAddressOf(),
        errorBlob.GetAddressOf()
    );

    if (FAILED(hr))
    {
        std::string message = "Shader compilation failed: " + resolvedPath.string();

        if (errorBlob != nullptr && errorBlob->GetBufferPointer() != nullptr)
        {
            const auto *errorText = static_cast<const char *>(errorBlob->GetBufferPointer());
            message += "\n";
            message.append(errorText, errorText + errorBlob->GetBufferSize());
        }

        throw std::runtime_error(message);
    }

    return shaderBlob;
}
