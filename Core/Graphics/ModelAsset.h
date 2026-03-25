#ifndef PINGPONG_MODELASSET_H
#define PINGPONG_MODELASSET_H

#include <DirectXCollision.h>

#include <filesystem>
#include <memory>
#include <directxtk/Model.h>

namespace DirectX
{
    inline namespace DX11
    {
        class IEffectFactory;
    }
}

class GraphicsDevice;

class ModelAsset final
{
public:
    bool LoadFromCmo(
        GraphicsDevice &graphics,
        DirectX::IEffectFactory &effectFactory,
        const std::filesystem::path &resolvedFilePath
    );

    [[nodiscard]] DirectX::Model *Get() const noexcept;

    [[nodiscard]] bool IsLoaded() const noexcept;

    [[nodiscard]] const std::filesystem::path &GetResolvedSourcePath() const noexcept;

    [[nodiscard]] DirectX::BoundingSphere GetMergedBoundingSphere() const noexcept;

private:
    std::unique_ptr<DirectX::Model> m_model{};
    std::filesystem::path m_resolvedSourcePath{};
};

#endif
