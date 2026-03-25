//
// Created by SyperOlao on 25.03.2026.
//

#include "Core/Assets/AssetCache.h"

#include "Core/Graphics/GraphicsDevice.h"
#include "Core/Graphics/ModelAsset.h"

#include <directxtk/Effects.h>

AssetCache::AssetCache() = default;

AssetCache::~AssetCache() = default;

void AssetCache::Initialize(GraphicsDevice &graphics)
{
    m_graphics = &graphics;
    m_effectFactory = std::make_unique<DirectX::EffectFactory>(graphics.GetDevice());
}

std::shared_ptr<ModelAsset> AssetCache::LoadModel(const std::wstring &path)
{
    const auto existing = m_models.find(path);
    if (existing != m_models.end())
    {
        return existing->second;
    }

    if (m_graphics == nullptr || m_effectFactory == nullptr)
    {
        return nullptr;
    }

    auto asset = std::make_shared<ModelAsset>();
    if (!asset->LoadFromCmo(*m_graphics, *m_effectFactory, path))
    {
        return nullptr;
    }

    m_models[path] = asset;
    return asset;
}
