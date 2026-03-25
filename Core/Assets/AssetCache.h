//
// Created by SyperOlao on 25.03.2026.
//

#ifndef PINGPONG_ASSETCACHE_H
#define PINGPONG_ASSETCACHE_H

#include <memory>
#include <string>
#include <unordered_map>

class GraphicsDevice;
class ModelAsset;

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

    [[nodiscard]] std::shared_ptr<ModelAsset> LoadModel(const std::wstring &path);

private:
    GraphicsDevice *m_graphics{nullptr};
    std::unique_ptr<DirectX::EffectFactory> m_effectFactory{};
    std::unordered_map<std::wstring, std::shared_ptr<ModelAsset> > m_models{};
};

#endif //PINGPONG_ASSETCACHE_H
