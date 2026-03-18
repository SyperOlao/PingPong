//
// Created by SyperOlao on 18.03.2026.
//

#ifndef MYPROJECT_SHADERCOMPILER_H
#define MYPROJECT_SHADERCOMPILER_H
#include <filesystem>
#include <string_view>
#include <wrl/client.h>
#include <d3dcompiler.h>

class ShaderCompiler {
public:
    static Microsoft::WRL::ComPtr<ID3DBlob> CompileFromFile(
        const std::filesystem::path &filePath,
        std::string_view entryPoint,
        std::string_view profile
    );

private:
    static std::filesystem::path ResolveSourcePath(const std::filesystem::path &filePath);
};


#endif //MYPROJECT_SHADERCOMPILER_H
