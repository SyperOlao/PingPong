#include "Core/Graphics/Rendering/Deferred/UIntRenderTarget2D.h"

#include "Core/Graphics/D3d11Helpers.h"
#include "Core/Graphics/GraphicsDevice.h"

#include <cstring>
#include <stdexcept>

bool UIntRenderTarget2D::IsCreated() const noexcept
{
    return m_texture != nullptr;
}

std::uint32_t UIntRenderTarget2D::GetWidth() const noexcept
{
    return m_width;
}

std::uint32_t UIntRenderTarget2D::GetHeight() const noexcept
{
    return m_height;
}

DXGI_FORMAT UIntRenderTarget2D::GetFormat() const noexcept
{
    return m_format;
}

void UIntRenderTarget2D::Create(
    ID3D11Device *const device,
    const std::uint32_t width,
    const std::uint32_t height,
    const DXGI_FORMAT format
)
{
    if (device == nullptr)
    {
        throw std::invalid_argument("UIntRenderTarget2D::Create requires a device.");
    }

    if (width == 0u || height == 0u)
    {
        throw std::invalid_argument("UIntRenderTarget2D::Create requires positive width and height.");
    }

    if (format == DXGI_FORMAT_UNKNOWN)
    {
        throw std::invalid_argument("UIntRenderTarget2D::Create requires a valid DXGI format.");
    }

    Destroy();
    m_format = format;
    CreateInternal(device, width, height);
}

void UIntRenderTarget2D::Destroy() noexcept
{
    m_shaderResourceView.Reset();
    m_renderTargetView.Reset();
    m_texture.Reset();
    m_width = 0u;
    m_height = 0u;
    m_format = DXGI_FORMAT_UNKNOWN;
}

void UIntRenderTarget2D::Resize(
    ID3D11Device *const device,
    const std::uint32_t width,
    const std::uint32_t height
)
{
    if (device == nullptr)
    {
        throw std::invalid_argument("UIntRenderTarget2D::Resize requires a device.");
    }

    if (width == 0u || height == 0u)
    {
        throw std::invalid_argument("UIntRenderTarget2D::Resize requires positive width and height.");
    }

    if (m_format == DXGI_FORMAT_UNKNOWN)
    {
        throw std::logic_error("UIntRenderTarget2D::Resize called before Create.");
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

ID3D11Texture2D *UIntRenderTarget2D::GetTexture() const noexcept
{
    return m_texture.Get();
}

ID3D11RenderTargetView *UIntRenderTarget2D::GetRenderTargetView() const noexcept
{
    return m_renderTargetView.Get();
}

ID3D11ShaderResourceView *UIntRenderTarget2D::GetShaderResourceView() const noexcept
{
    return m_shaderResourceView.Get();
}

void UIntRenderTarget2D::Clear(GraphicsDevice &graphics, const std::uint32_t clearValue) const
{
    if (m_renderTargetView == nullptr)
    {
        throw std::logic_error("UIntRenderTarget2D::Clear called before Create.");
    }

    ID3D11DeviceContext *const context = graphics.GetImmediateContext();
    if (context == nullptr)
    {
        throw std::logic_error("UIntRenderTarget2D::Clear requires a device context.");
    }

    float clearAsFloat[4]{};
    std::memcpy(clearAsFloat, &clearValue, sizeof(std::uint32_t));
    context->ClearRenderTargetView(m_renderTargetView.Get(), clearAsFloat);
}

void UIntRenderTarget2D::CreateInternal(ID3D11Device *const device, const std::uint32_t width, const std::uint32_t height)
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
        "UIntRenderTarget2D failed to create texture."
    );

    D3d11Helpers::ThrowIfFailed(
        device->CreateRenderTargetView(m_texture.Get(), nullptr, m_renderTargetView.GetAddressOf()),
        "UIntRenderTarget2D failed to create render target view."
    );

    D3d11Helpers::ThrowIfFailed(
        device->CreateShaderResourceView(m_texture.Get(), nullptr, m_shaderResourceView.GetAddressOf()),
        "UIntRenderTarget2D failed to create shader resource view."
    );

    m_width = width;
    m_height = height;
}
