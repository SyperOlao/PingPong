#include "Core/Graphics/ModelAsset.h"

#include "Core/Graphics/GraphicsDevice.h"

#include <directxtk/Effects.h>
#include <directxtk/Model.h>

bool ModelAsset::LoadFromCmo(
    GraphicsDevice &graphics,
    DirectX::IEffectFactory &effectFactory,
    const std::filesystem::path &resolvedFilePath
)
{
    m_model.reset();
    m_resolvedSourcePath.clear();

    m_model = DirectX::Model::CreateFromCMO(graphics.GetDevice(), resolvedFilePath.c_str(), effectFactory);
    if (m_model == nullptr)
    {
        return false;
    }

    m_resolvedSourcePath = resolvedFilePath;
    return true;
}

DirectX::Model *ModelAsset::Get() const noexcept
{
    return m_model.get();
}

bool ModelAsset::IsLoaded() const noexcept
{
    return m_model != nullptr;
}

const std::filesystem::path &ModelAsset::GetResolvedSourcePath() const noexcept
{
    return m_resolvedSourcePath;
}

DirectX::BoundingSphere ModelAsset::GetMergedBoundingSphere() const noexcept
{
    if (m_model == nullptr || m_model->meshes.empty())
    {
        return {};
    }

    DirectX::BoundingSphere merged = m_model->meshes[0]->boundingSphere;
    for (size_t meshIndex = 1; meshIndex < m_model->meshes.size(); ++meshIndex)
    {
        DirectX::BoundingSphere combined{};
        DirectX::BoundingSphere::CreateMerged(combined, merged, m_model->meshes[meshIndex]->boundingSphere);
        merged = combined;
    }

    return merged;
}
