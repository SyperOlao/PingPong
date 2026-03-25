#include "Core/Graphics/Rendering/Deferred/GBufferResources.h"

#include "Core/Graphics/GraphicsDevice.h"

#include <stdexcept>

bool GBufferResources::IsCreated() const noexcept
{
    return m_albedoOcclusion.IsCreated() && m_sceneDepth.IsCreated();
}

std::uint32_t GBufferResources::GetWidth() const noexcept
{
    return m_albedoOcclusion.GetWidth();
}

std::uint32_t GBufferResources::GetHeight() const noexcept
{
    return m_albedoOcclusion.GetHeight();
}

void GBufferResources::Create(ID3D11Device *const device, const std::uint32_t width, const std::uint32_t height)
{
    if (device == nullptr)
    {
        throw std::invalid_argument("GBufferResources::Create requires a device.");
    }

    if (width == 0u || height == 0u)
    {
        throw std::invalid_argument("GBufferResources::Create requires positive width and height.");
    }

    Destroy();

    m_albedoOcclusion.Create(device, width, height, kGBufferAlbedoOcclusionFormat);
    m_normal.Create(device, width, height, kGBufferNormalFormat);
    m_material.Create(device, width, height, kGBufferMaterialFormat);
    m_sceneDepth.Create(device, width, height);
}

void GBufferResources::Destroy() noexcept
{
    m_sceneDepth.Destroy();
    m_material.Destroy();
    m_normal.Destroy();
    m_albedoOcclusion.Destroy();
}

void GBufferResources::Resize(ID3D11Device *const device, const std::uint32_t width, const std::uint32_t height)
{
    if (device == nullptr)
    {
        throw std::invalid_argument("GBufferResources::Resize requires a device.");
    }

    if (width == 0u || height == 0u)
    {
        throw std::invalid_argument("GBufferResources::Resize requires positive width and height.");
    }

    if (!m_albedoOcclusion.IsCreated())
    {
        Create(device, width, height);
        return;
    }

    m_albedoOcclusion.Resize(device, width, height);
    m_normal.Resize(device, width, height);
    m_material.Resize(device, width, height);
    m_sceneDepth.Resize(device, width, height);
}

ColorRenderTarget2D &GBufferResources::GetAlbedoOcclusionTarget() noexcept
{
    return m_albedoOcclusion;
}

const ColorRenderTarget2D &GBufferResources::GetAlbedoOcclusionTarget() const noexcept
{
    return m_albedoOcclusion;
}

ColorRenderTarget2D &GBufferResources::GetNormalTarget() noexcept
{
    return m_normal;
}

const ColorRenderTarget2D &GBufferResources::GetNormalTarget() const noexcept
{
    return m_normal;
}

ColorRenderTarget2D &GBufferResources::GetMaterialTarget() noexcept
{
    return m_material;
}

const ColorRenderTarget2D &GBufferResources::GetMaterialTarget() const noexcept
{
    return m_material;
}

DepthRenderTarget2D &GBufferResources::GetSceneDepthTarget() noexcept
{
    return m_sceneDepth;
}

const DepthRenderTarget2D &GBufferResources::GetSceneDepthTarget() const noexcept
{
    return m_sceneDepth;
}

void GBufferResources::BindGeometryPassTargets(GraphicsDevice &graphics) const
{
    if (!IsCreated())
    {
        throw std::logic_error("GBufferResources::BindGeometryPassTargets called before Create.");
    }

    ID3D11RenderTargetView *const renderTargets[kGBufferColorSurfaceCount] = {
        m_albedoOcclusion.GetRenderTargetView(),
        m_normal.GetRenderTargetView(),
        m_material.GetRenderTargetView()
    };

    graphics.BindRenderTargetsAndDepth(renderTargets, kGBufferColorSurfaceCount, m_sceneDepth.GetDepthStencilView());
}

void GBufferResources::ClearGeometryPassTargets(
    GraphicsDevice &graphics,
    const Color &albedoOcclusionClear,
    const Color &normalClear,
    const Color &materialClear,
    const float depthClearValue
) const
{
    if (!IsCreated())
    {
        throw std::logic_error("GBufferResources::ClearGeometryPassTargets called before Create.");
    }

    m_albedoOcclusion.Clear(graphics, albedoOcclusionClear);
    m_normal.Clear(graphics, normalClear);
    m_material.Clear(graphics, materialClear);
    graphics.ClearDepthStencilView(m_sceneDepth.GetDepthStencilView(), depthClearValue, 0u);
}
