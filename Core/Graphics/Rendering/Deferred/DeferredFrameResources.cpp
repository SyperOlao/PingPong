#include "Core/Graphics/Rendering/Deferred/DeferredFrameResources.h"

#include "Core/Graphics/GraphicsDevice.h"

#include <d3d11.h>

#include <stdexcept>

bool DeferredFrameResources::IsCreated() const noexcept
{
    return m_gbuffer.IsCreated() && m_lightAccumulation.IsCreated();
}

void DeferredFrameResources::CreateOrResize(GraphicsDevice &graphics)
{
    ID3D11Device *const device = graphics.GetDevice();
    if (device == nullptr)
    {
        throw std::logic_error("DeferredFrameResources::CreateOrResize requires a valid device.");
    }

    const int WidthPixels = graphics.GetWidth();
    const int HeightPixels = graphics.GetHeight();
    if (WidthPixels <= 0 || HeightPixels <= 0)
    {
        throw std::invalid_argument("DeferredFrameResources::CreateOrResize requires positive surface dimensions.");
    }

    const std::uint32_t Width = static_cast<std::uint32_t>(WidthPixels);
    const std::uint32_t Height = static_cast<std::uint32_t>(HeightPixels);

    if (!m_gbuffer.IsCreated())
    {
        m_gbuffer.Create(device, Width, Height);
    }
    else
    {
        m_gbuffer.Resize(device, Width, Height);
    }

    if (!m_lightAccumulation.IsCreated())
    {
        m_lightAccumulation.Create(device, Width, Height, DXGI_FORMAT_R11G11B10_FLOAT);
    }
    else
    {
        m_lightAccumulation.Resize(device, Width, Height);
    }
}

void DeferredFrameResources::Destroy() noexcept
{
    m_lightAccumulation.Destroy();
    m_gbuffer.Destroy();
}

GBufferResources &DeferredFrameResources::GetGBufferResources() noexcept
{
    return m_gbuffer;
}

const GBufferResources &DeferredFrameResources::GetGBufferResources() const noexcept
{
    return m_gbuffer;
}

ColorRenderTarget2D &DeferredFrameResources::GetLightAccumulationTarget() noexcept
{
    return m_lightAccumulation;
}

const ColorRenderTarget2D &DeferredFrameResources::GetLightAccumulationTarget() const noexcept
{
    return m_lightAccumulation;
}
