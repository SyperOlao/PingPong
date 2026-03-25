#ifndef PINGPONG_ASSETCACHE_H
#define PINGPONG_ASSETCACHE_H

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

class GraphicsDevice;
class ModelAsset;
class Texture2DAsset;

namespace DirectX
{
    inline namespace DX11
    {
        class EffectFactory;
    }
}

class AssetCache final
{
public:
    AssetCache();

    ~AssetCache();

    AssetCache(const AssetCache &) = delete;

    AssetCache &operator=(const AssetCache &) = delete;

    AssetCache(AssetCache &&) = delete;

    AssetCache &operator=(AssetCache &&) = delete;

    void Initialize(GraphicsDevice &graphics);

    [[nodiscard]] bool IsInitialized() const noexcept;

    [[nodiscard]] std::shared_ptr<ModelAsset> LoadModel(const std::filesystem::path &contentRelativeOrAbsolutePath);

    [[nodiscard]] std::shared_ptr<Texture2DAsset> LoadTexture2DWic(const std::filesystem::path &contentRelativeOrAbsolutePath);

    void UnloadAll();

private:
    GraphicsDevice *m_graphics{nullptr};
    std::unique_ptr<DirectX::EffectFactory> m_effectFactory{};
    std::unordered_map<std::wstring, std::shared_ptr<ModelAsset>> m_models{};
    std::unordered_map<std::wstring, std::shared_ptr<Texture2DAsset>> m_textures{};
};

#endif
