#ifndef PINGPONG_SHADERCOMPILER_H
#define PINGPONG_SHADERCOMPILER_H

#include <filesystem>
#include <string_view>
#include <wrl/client.h>
#include <d3dcompiler.h>

class ShaderCompiler final
{
public:
    [[nodiscard]] static Microsoft::WRL::ComPtr<ID3DBlob> CompileFromFile(
        const std::filesystem::path &filePath,
        std::string_view entryPoint,
        std::string_view profile
    );

private:
    ShaderCompiler() = default;
};

#endif
