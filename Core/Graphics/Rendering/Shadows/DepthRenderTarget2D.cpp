#include "Core/Graphics/Rendering/Shadows/DepthRenderTarget2D.h"

#include "Core/Graphics/D3d11Helpers.h"

#include <stdexcept>

bool DepthRenderTarget2D::IsCreated() const noexcept
{
    return m_texture != nullptr;
}

std::uint32_t DepthRenderTarget2D::GetWidth() const noexcept
{
    return m_width;
}

std::uint32_t DepthRenderTarget2D::GetHeight() const noexcept
{
    return m_height;
}

void DepthRenderTarget2D::Create(ID3D11Device *const device, const std::uint32_t width, const std::uint32_t height)
{
    if (device == nullptr)
    {
        throw std::invalid_argument("DepthRenderTarget2D::Create requires a device.");
    }

    if (width == 0u || height == 0u)
    {
        throw std::invalid_argument("DepthRenderTarget2D::Create requires positive width and height.");
    }

    Destroy();
    CreateInternal(device, width, height);
}

void DepthRenderTarget2D::Destroy() noexcept
{
    m_shaderResourceView.Reset();
    m_depthStencilView.Reset();
    m_texture.Reset();
    m_width = 0u;
    m_height = 0u;
}

void DepthRenderTarget2D::Resize(ID3D11Device *const device, const std::uint32_t width, const std::uint32_t height)
{
    if (device == nullptr)
    {
        throw std::invalid_argument("DepthRenderTarget2D::Resize requires a device.");
    }

    if (width == 0u || height == 0u)
    {
        throw std::invalid_argument("DepthRenderTarget2D::Resize requires positive width and height.");
    }

    if (width == m_width && height == m_height && m_texture != nullptr)
    {
        return;
    }

    Destroy();
    CreateInternal(device, width, height);
}

ID3D11Texture2D *DepthRenderTarget2D::GetTexture() const noexcept
{
    return m_texture.Get();
}

ID3D11DepthStencilView *DepthRenderTarget2D::GetDepthStencilView() const noexcept
{
    return m_depthStencilView.Get();
}

ID3D11ShaderResourceView *DepthRenderTarget2D::GetShaderResourceView() const noexcept
{
    return m_shaderResourceView.Get();
}

void DepthRenderTarget2D::CreateInternal(ID3D11Device *const device, const std::uint32_t width, const std::uint32_t height)
{
    D3D11_TEXTURE2D_DESC textureDescription{};
    textureDescription.Width = width;
    textureDescription.Height = height;
    textureDescription.MipLevels = 1u;
    textureDescription.ArraySize = 1u;
    textureDescription.Format = DXGI_FORMAT_R32_TYPELESS;
    textureDescription.SampleDesc.Count = 1u;
    textureDescription.SampleDesc.Quality = 0u;
    textureDescription.Usage = D3D11_USAGE_DEFAULT;
    textureDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    textureDescription.CPUAccessFlags = 0u;
    textureDescription.MiscFlags = 0u;

    D3d11Helpers::ThrowIfFailed(
        device->CreateTexture2D(&textureDescription, nullptr, m_texture.GetAddressOf()),
        "DepthRenderTarget2D failed to create depth texture."
    );

    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDescription{};
    depthStencilViewDescription.Format = DXGI_FORMAT_D32_FLOAT;
    depthStencilViewDescription.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDescription.Flags = 0u;
    depthStencilViewDescription.Texture2D.MipSlice = 0u;

    D3d11Helpers::ThrowIfFailed(
        device->CreateDepthStencilView(m_texture.Get(), &depthStencilViewDescription, m_depthStencilView.GetAddressOf()),
        "DepthRenderTarget2D failed to create depth stencil view."
    );

    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDescription{};
    shaderResourceViewDescription.Format = DXGI_FORMAT_R32_FLOAT;
    shaderResourceViewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDescription.Texture2D.MostDetailedMip = 0u;
    shaderResourceViewDescription.Texture2D.MipLevels = 1u;

    D3d11Helpers::ThrowIfFailed(
        device->CreateShaderResourceView(m_texture.Get(), &shaderResourceViewDescription, m_shaderResourceView.GetAddressOf()),
        "DepthRenderTarget2D failed to create shader resource view."
    );

    m_width = width;
    m_height = height;
}
