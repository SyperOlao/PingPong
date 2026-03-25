#ifndef PINGPONG_DEFERREDFRAMERESOURCES_H
#define PINGPONG_DEFERREDFRAMERESOURCES_H

#include "Core/Graphics/Rendering/Deferred/ColorRenderTarget2D.h"
#include "Core/Graphics/Rendering/Deferred/GBufferResources.h"

class GraphicsDevice;

class DeferredFrameResources final
{
public:
    [[nodiscard]] bool IsCreated() const noexcept;

    void CreateOrResize(GraphicsDevice &graphics);

    void Destroy() noexcept;

    [[nodiscard]] GBufferResources &GetGBufferResources() noexcept;

    [[nodiscard]] const GBufferResources &GetGBufferResources() const noexcept;

    [[nodiscard]] ColorRenderTarget2D &GetLightAccumulationTarget() noexcept;

    [[nodiscard]] const ColorRenderTarget2D &GetLightAccumulationTarget() const noexcept;

private:
    GBufferResources m_gbuffer{};
    ColorRenderTarget2D m_lightAccumulation{};
};

#endif
