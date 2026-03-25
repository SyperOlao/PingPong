#include "Core/Graphics/Rendering/FrameRenderer.h"

#include "Core/Graphics/GraphicsDevice.h"
#include "Core/Graphics/Rendering/Deferred/ColorRenderTarget2D.h"
#include "Core/Graphics/Rendering/Deferred/GBufferResources.h"

#include <stdexcept>

void FrameRenderer::Initialize(GraphicsDevice &graphics) noexcept
{
    m_graphics = &graphics;
}

void FrameRenderer::BeginFrame(const Color &clearColor)
{
    if (m_graphics == nullptr)
    {
        throw std::logic_error("FrameRenderer::BeginFrame called before Initialize.");
    }

    m_activePass = RenderPassKind::None;
    m_graphics->BeginFrame(clearColor);
}

void FrameRenderer::EndFrame(const bool vSync) const
{
    if (m_graphics == nullptr)
    {
        throw std::logic_error("FrameRenderer::EndFrame called before Initialize.");
    }

    m_graphics->EndFrame(vSync);
}

void FrameRenderer::BindMainPassTargets() const
{
    if (m_graphics == nullptr)
    {
        throw std::logic_error("FrameRenderer::BindMainPassTargets called before Initialize.");
    }

    m_graphics->BindMainRenderTargets();
    m_graphics->SetMainViewport();
}

void FrameRenderer::BindDeferredGeometryPass(GBufferResources &gBufferResources)
{
    if (m_graphics == nullptr)
    {
        throw std::logic_error("FrameRenderer::BindDeferredGeometryPass called before Initialize.");
    }

    EnterPass(RenderPassKind::DeferredGeometry);
    gBufferResources.BindGeometryPassTargets(*m_graphics);
    m_graphics->SetViewportPixels(
        static_cast<float>(gBufferResources.GetWidth()),
        static_cast<float>(gBufferResources.GetHeight())
    );
}

void FrameRenderer::BindDeferredLightingAccumulation(ColorRenderTarget2D &lightAccumulation)
{
    if (m_graphics == nullptr)
    {
        throw std::logic_error("FrameRenderer::BindDeferredLightingAccumulation called before Initialize.");
    }

    EnterPass(RenderPassKind::DeferredLighting);
    lightAccumulation.BindAsSingleRenderTarget(*m_graphics);
    m_graphics->SetViewportPixels(
        static_cast<float>(lightAccumulation.GetWidth()),
        static_cast<float>(lightAccumulation.GetHeight())
    );
}

void FrameRenderer::EnterPass(const RenderPassKind passKind) noexcept
{
    m_activePass = passKind;
}

void FrameRenderer::LeavePass() noexcept
{
    m_activePass = RenderPassKind::None;
}

RenderPassKind FrameRenderer::GetActivePass() const noexcept
{
    return m_activePass;
}
