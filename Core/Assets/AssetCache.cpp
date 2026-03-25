#include "Core/Assets/AssetCache.h"

#include "Core/Assets/AssetPathResolver.h"
#include "Core/Graphics/GraphicsDevice.h"
#include "Core/Graphics/ModelAsset.h"
#include "Core/Graphics/Texture2DAsset.h"

#include <directxtk/Effects.h>

AssetCache::AssetCache() = default;

AssetCache::~AssetCache() = default;

void AssetCache::Initialize(GraphicsDevice &graphics)
{
    m_graphics = &graphics;
    m_effectFactory = std::make_unique<DirectX::EffectFactory>(graphics.GetDevice());
}

bool AssetCache::IsInitialized() const noexcept
{
    return m_graphics != nullptr && m_effectFactory != nullptr;
}

std::shared_ptr<ModelAsset> AssetCache::LoadModel(const std::filesystem::path &contentRelativeOrAbsolutePath)
{
    if (!IsInitialized())
    {
        return nullptr;
    }

    const std::filesystem::path resolvedPath = AssetPathResolver::Resolve(contentRelativeOrAbsolutePath);
    const std::wstring cacheKey = AssetPathResolver::MakeCacheKey(resolvedPath);

    const auto existing = m_models.find(cacheKey);
    if (existing != m_models.end())
    {
        return existing->second;
    }

    auto asset = std::make_shared<ModelAsset>();
    if (!asset->LoadFromCmo(*m_graphics, *m_effectFactory, resolvedPath))
    {
        return nullptr;
    }

    m_models[cacheKey] = asset;
    return asset;
}

std::shared_ptr<Texture2DAsset> AssetCache::LoadTexture2DWic(const std::filesystem::path &contentRelativeOrAbsolutePath)
{
    if (!IsInitialized())
    {
        return nullptr;
    }

    const std::filesystem::path resolvedPath = AssetPathResolver::Resolve(contentRelativeOrAbsolutePath);
    const std::wstring cacheKey = AssetPathResolver::MakeCacheKey(resolvedPath);

    const auto existing = m_textures.find(cacheKey);
    if (existing != m_textures.end())
    {
        return existing->second;
    }

    auto asset = std::make_shared<Texture2DAsset>();
    if (!asset->LoadFromWicFile(*m_graphics, resolvedPath))
    {
        return nullptr;
    }

    m_textures[cacheKey] = asset;
    return asset;
}

void AssetCache::UnloadAll()
{
    m_models.clear();
    m_textures.clear();
}
