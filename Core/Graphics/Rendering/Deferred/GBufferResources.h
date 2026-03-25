#ifndef PINGPONG_GBUFFERRESOURCES_H
#define PINGPONG_GBUFFERRESOURCES_H

#include "Core/Graphics/Color.h"
#include "Core/Graphics/Rendering/Deferred/GBufferLayout.h"
#include "Core/Graphics/Rendering/Deferred/ColorRenderTarget2D.h"
#include "Core/Graphics/Rendering/Shadows/DepthRenderTarget2D.h"

#include <cstdint>

class GraphicsDevice;

class GBufferResources final
{
public:
    [[nodiscard]] bool IsCreated() const noexcept;

    [[nodiscard]] std::uint32_t GetWidth() const noexcept;

    [[nodiscard]] std::uint32_t GetHeight() const noexcept;

    void Create(ID3D11Device *device, std::uint32_t width, std::uint32_t height);

    void Destroy() noexcept;

    void Resize(ID3D11Device *device, std::uint32_t width, std::uint32_t height);

    [[nodiscard]] ColorRenderTarget2D &GetAlbedoOcclusionTarget() noexcept;

    [[nodiscard]] const ColorRenderTarget2D &GetAlbedoOcclusionTarget() const noexcept;

    [[nodiscard]] ColorRenderTarget2D &GetNormalTarget() noexcept;

    [[nodiscard]] const ColorRenderTarget2D &GetNormalTarget() const noexcept;

    [[nodiscard]] ColorRenderTarget2D &GetMaterialTarget() noexcept;

    [[nodiscard]] const ColorRenderTarget2D &GetMaterialTarget() const noexcept;

    [[nodiscard]] DepthRenderTarget2D &GetSceneDepthTarget() noexcept;

    [[nodiscard]] const DepthRenderTarget2D &GetSceneDepthTarget() const noexcept;

    void BindGeometryPassTargets(GraphicsDevice &graphics) const;

    void ClearGeometryPassTargets(
        GraphicsDevice &graphics,
        const Color &albedoOcclusionClear,
        const Color &normalClear,
        const Color &materialClear,
        float depthClearValue
    ) const;

private:
    ColorRenderTarget2D m_albedoOcclusion{};
    ColorRenderTarget2D m_normal{};
    ColorRenderTarget2D m_material{};
    DepthRenderTarget2D m_sceneDepth{};
};

#endif
