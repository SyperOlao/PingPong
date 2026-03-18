//
// Created by SyperOlao on 18.03.2026.
//

#include "ShaderCompiler.h"
#include <Windows.h>
#include <d3dcompiler.h>

#include <filesystem>
#include <stdexcept>
#include <string>

namespace
{
    [[nodiscard]] std::filesystem::path GetExecutableDirectory()
    {
        std::wstring buffer(MAX_PATH, L'\0');

        DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
        if (length == 0)
        {
            throw std::runtime_error("GetModuleFileNameW failed.");
        }

        while (length == buffer.size())
        {
            buffer.resize(buffer.size() * 2);
            length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));

            if (length == 0)
            {
                throw std::runtime_error("GetModuleFileNameW failed.");
            }
        }

        buffer.resize(length);
        return std::filesystem::path(buffer).parent_path();
    }
}

Microsoft::WRL::ComPtr<ID3DBlob> ShaderCompiler::CompileFromFile(
    const std::filesystem::path& filePath,
    const std::string_view entryPoint,
    const std::string_view profile
)
{
    const std::filesystem::path resolvedPath = ResolveSourcePath(filePath);

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
        entryPoint.data(),
        profile.data(),
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
            const auto* errorText = static_cast<const char*>(errorBlob->GetBufferPointer());
            message += "\n";
            message.append(errorText, errorText + errorBlob->GetBufferSize());
        }

        throw std::runtime_error(message);
    }

    return shaderBlob;
}

std::filesystem::path ShaderCompiler::ResolveSourcePath(const std::filesystem::path& filePath)
{
    if (filePath.empty())
    {
        throw std::invalid_argument("ShaderCompiler::ResolveSourcePath received an empty path.");
    }

    if (std::filesystem::exists(filePath))
    {
        return std::filesystem::absolute(filePath);
    }

    std::filesystem::path current = GetExecutableDirectory();

    for (int i = 0; i < 8; ++i)
    {
        const std::filesystem::path candidate = current / filePath;
        if (std::filesystem::exists(candidate))
        {
            return std::filesystem::weakly_canonical(candidate);
        }

        if (!current.has_parent_path())
        {
            break;
        }

        const auto parent = current.parent_path();
        if (parent == current)
        {
            break;
        }

        current = parent;
    }

    throw std::runtime_error("Shader file not found: " + filePath.string());
}