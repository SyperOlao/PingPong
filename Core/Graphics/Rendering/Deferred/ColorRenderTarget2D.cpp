#include "Core/Graphics/Rendering/Deferred/ColorRenderTarget2D.h"

#include "Core/Graphics/D3d11Helpers.h"
#include "Core/Graphics/GraphicsDevice.h"

#include <stdexcept>

bool ColorRenderTarget2D::IsCreated() const noexcept
{
    return m_texture != nullptr;
}

std::uint32_t ColorRenderTarget2D::GetWidth() const noexcept
{
    return m_width;
}

std::uint32_t ColorRenderTarget2D::GetHeight() const noexcept
{
    return m_height;
}

DXGI_FORMAT ColorRenderTarget2D::GetFormat() const noexcept
{
    return m_format;
}

void ColorRenderTarget2D::Create(
    ID3D11Device *const device,
    const std::uint32_t width,
    const std::uint32_t height,
    const DXGI_FORMAT format
)
{
    if (device == nullptr)
    {
        throw std::invalid_argument("ColorRenderTarget2D::Create requires a device.");
    }

    if (width == 0u || height == 0u)
    {
        throw std::invalid_argument("ColorRenderTarget2D::Create requires positive width and height.");
    }

    if (format == DXGI_FORMAT_UNKNOWN)
    {
        throw std::invalid_argument("ColorRenderTarget2D::Create requires a valid DXGI format.");
    }

    Destroy();
    m_format = format;
    CreateInternal(device, width, height);
}

void ColorRenderTarget2D::Destroy() noexcept
{
    m_shaderResourceView.Reset();
    m_renderTargetView.Reset();
    m_texture.Reset();
    m_width = 0u;
    m_height = 0u;
    m_format = DXGI_FORMAT_UNKNOWN;
}

void ColorRenderTarget2D::Resize(
    ID3D11Device *const device,
    const std::uint32_t width,
    const std::uint32_t height
)
{
    if (device == nullptr)
    {
        throw std::invalid_argument("ColorRenderTarget2D::Resize requires a device.");
    }

    if (width == 0u || height == 0u)
    {
        throw std::invalid_argument("ColorRenderTarget2D::Resize requires positive width and height.");
    }

    if (m_format == DXGI_FORMAT_UNKNOWN)
    {
        throw std::logic_error("ColorRenderTarget2D::Resize called before Create.");
    }

    if (width == m_width && height == m_height && m_texture != nullptr)
    {
        return;
    }

    const DXGI_FORMAT PreservedFormat = m_format;
    Destroy();
    m_format = PreservedFormat;
    CreateInternal(device, width, height);
}

ID3D11Texture2D *ColorRenderTarget2D::GetTexture() const noexcept
{
    return m_texture.Get();
}

ID3D11RenderTargetView *ColorRenderTarget2D::GetRenderTargetView() const noexcept
{
    return m_renderTargetView.Get();
}

ID3D11ShaderResourceView *ColorRenderTarget2D::GetShaderResourceView() const noexcept
{
    return m_shaderResourceView.Get();
}

void ColorRenderTarget2D::BindAsSingleRenderTarget(GraphicsDevice &graphics) const
{
    if (m_renderTargetView == nullptr)
    {
        throw std::logic_error("ColorRenderTarget2D::BindAsSingleRenderTarget called before Create.");
    }

    graphics.BindSingleRenderTarget(m_renderTargetView.Get());
}

void ColorRenderTarget2D::Clear(GraphicsDevice &graphics, const Color &clearColor) const
{
    if (m_renderTargetView == nullptr)
    {
        throw std::logic_error("ColorRenderTarget2D::Clear called before Create.");
    }

    graphics.ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
}

void ColorRenderTarget2D::CreateInternal(ID3D11Device *const device, const std::uint32_t width, const std::uint32_t height)
{
    D3D11_TEXTURE2D_DESC textureDescription{};
    textureDescription.Width = width;
    textureDescription.Height = height;
    textureDescription.MipLevels = 1u;
    textureDescription.ArraySize = 1u;
    textureDescription.Format = m_format;
    textureDescription.SampleDesc.Count = 1u;
    textureDescription.SampleDesc.Quality = 0u;
    textureDescription.Usage = D3D11_USAGE_DEFAULT;
    textureDescription.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    textureDescription.CPUAccessFlags = 0u;
    textureDescription.MiscFlags = 0u;

    D3d11Helpers::ThrowIfFailed(
        device->CreateTexture2D(&textureDescription, nullptr, m_texture.GetAddressOf()),
        "ColorRenderTarget2D failed to create texture."
    );

    D3d11Helpers::ThrowIfFailed(
        device->CreateRenderTargetView(m_texture.Get(), nullptr, m_renderTargetView.GetAddressOf()),
        "ColorRenderTarget2D failed to create render target view."
    );

    D3d11Helpers::ThrowIfFailed(
        device->CreateShaderResourceView(m_texture.Get(), nullptr, m_shaderResourceView.GetAddressOf()),
        "ColorRenderTarget2D failed to create shader resource view."
    );

    m_width = width;
    m_height = height;
}
