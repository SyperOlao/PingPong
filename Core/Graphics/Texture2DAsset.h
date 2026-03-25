#ifndef PINGPONG_TEXTURE2DASSET_H
#define PINGPONG_TEXTURE2DASSET_H

#include <d3d11.h>
#include <filesystem>
#include <wrl/client.h>

class GraphicsDevice;

class Texture2DAsset final
{
public:
    Texture2DAsset() = default;

    ~Texture2DAsset() = default;

    Texture2DAsset(const Texture2DAsset &) = delete;

    Texture2DAsset &operator=(const Texture2DAsset &) = delete;

    Texture2DAsset(Texture2DAsset &&) = delete;

    Texture2DAsset &operator=(Texture2DAsset &&) = delete;

    bool LoadFromWicFile(GraphicsDevice &graphics, const std::filesystem::path &resolvedFilePath);

    [[nodiscard]] ID3D11ShaderResourceView *GetShaderResourceView() const noexcept;

    [[nodiscard]] const std::filesystem::path &GetSourcePath() const noexcept;

private:
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderResourceView{};
    std::filesystem::path m_resolvedSourcePath{};
};

#endif
