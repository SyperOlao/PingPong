#ifndef PINGPONG_FRAMERENDERER_H
#define PINGPONG_FRAMERENDERER_H

#include "Core/Graphics/Rendering/FrameRenderTypes.h"
#include "Core/Graphics/Color.h"

class GraphicsDevice;
class GBufferResources;
class ColorRenderTarget2D;

class FrameRenderer final
{
public:
    void Initialize(GraphicsDevice &graphics) noexcept;

    void BeginFrame(const Color &clearColor);

    void EndFrame(bool vSync) const;

    void BindMainPassTargets() const;

    void BindDeferredGeometryPass(GBufferResources &gBufferResources);

    void BindDeferredLightingAccumulation(ColorRenderTarget2D &lightAccumulation);

    void EnterPass(RenderPassKind passKind) noexcept;

    void LeavePass() noexcept;

    [[nodiscard]] RenderPassKind GetActivePass() const noexcept;

private:
    GraphicsDevice *m_graphics{nullptr};
    RenderPassKind m_activePass{RenderPassKind::None};
};

#endif
