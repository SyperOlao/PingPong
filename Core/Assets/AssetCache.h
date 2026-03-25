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

class AssetCache final {
public:
    void Initialize(GraphicsDevice &graphics);

    [[nodiscard]] std::shared_ptr<ModelAsset> LoadModel(const std::wstring &path);

private:
    GraphicsDevice *m_graphics{nullptr};
    std::unordered_map<std::wstring, std::shared_ptr<ModelAsset> > m_models;
};

#endif //PINGPONG_ASSETCACHE_H
