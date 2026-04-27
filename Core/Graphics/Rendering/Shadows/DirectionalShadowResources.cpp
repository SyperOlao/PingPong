#include "Core/Graphics/Rendering/Shadows/DirectionalShadowResources.h"

#include "Core/Graphics/D3d11Helpers.h"
#include "Core/Graphics/GraphicsDevice.h"

#include <cstring>
#include <stdexcept>

void DirectionalShadowResources::Initialize(GraphicsDevice &graphicsDevice)
{
    ID3D11Device *const device = graphicsDevice.GetDevice();
    if (device == nullptr)
    {
        throw std::logic_error("DirectionalShadowResources::Initialize requires a valid device.");
    }

    m_comparisonSampler.Create(device);
    m_shadowPassRasterizer.Create(device, D3D11_CULL_NONE, 64, 0.35f, 0.0f);
    m_shadowMap.Create(device, m_shadowAtlasSizePixels, m_shadowAtlasSizePixels);
    CreateConstantBuffers(device);
}

void DirectionalShadowResources::Shutdown() noexcept
{
    m_shadowSamplingConstantBuffer.Reset();
    m_shadowCascadeConstantBuffer.Reset();
    m_shadowMap.Destroy();
    m_shadowPassRasterizer.Destroy();
    m_comparisonSampler.Destroy();
}

void DirectionalShadowResources::Resize(GraphicsDevice &graphicsDevice)
{
    ID3D11Device *const device = graphicsDevice.GetDevice();
    if (device == nullptr)
    {
        return;
    }

    m_shadowMap.Resize(device, m_shadowAtlasSizePixels, m_shadowAtlasSizePixels);
}

void DirectionalShadowResources::UploadShadowCascadeConstants(
    ID3D11DeviceContext *const deviceContext,
    const ShadowCascadeConstantsGpu &constants
) const
{
    if (deviceContext == nullptr || m_shadowCascadeConstantBuffer == nullptr)
    {
        throw std::logic_error("DirectionalShadowResources::UploadShadowCascadeConstants requires valid context and buffer.");
    }

    D3D11_MAPPED_SUBRESOURCE mapped{};
    D3d11Helpers::ThrowIfFailed(
        deviceContext->Map(m_shadowCascadeConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped),
        "DirectionalShadowResources failed to map cascade constant buffer."
    );
    std::memcpy(mapped.pData, &constants, sizeof(ShadowCascadeConstantsGpu));
    deviceContext->Unmap(m_shadowCascadeConstantBuffer.Get(), 0);
}

void DirectionalShadowResources::UploadShadowSamplingConstants(
    ID3D11DeviceContext *const deviceContext,
    const ShadowSamplingGpuConstants &constants
) const
{
    if (deviceContext == nullptr || m_shadowSamplingConstantBuffer == nullptr)
    {
        throw std::logic_error("DirectionalShadowResources::UploadShadowSamplingConstants requires valid context and buffer.");
    }

    D3D11_MAPPED_SUBRESOURCE mapped{};
    D3d11Helpers::ThrowIfFailed(
        deviceContext->Map(m_shadowSamplingConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped),
        "DirectionalShadowResources failed to map sampling constant buffer."
    );
    std::memcpy(mapped.pData, &constants, sizeof(ShadowSamplingGpuConstants));
    deviceContext->Unmap(m_shadowSamplingConstantBuffer.Get(), 0);
}

DepthRenderTarget2D &DirectionalShadowResources::GetShadowMap() noexcept
{
    return m_shadowMap;
}

const DepthRenderTarget2D &DirectionalShadowResources::GetShadowMap() const noexcept
{
    return m_shadowMap;
}

ID3D11Buffer *DirectionalShadowResources::GetShadowCascadeConstantBuffer() const noexcept
{
    return m_shadowCascadeConstantBuffer.Get();
}

ID3D11Buffer *DirectionalShadowResources::GetShadowSamplingConstantBuffer() const noexcept
{
    return m_shadowSamplingConstantBuffer.Get();
}

ID3D11SamplerState *DirectionalShadowResources::GetShadowComparisonSampler() const noexcept
{
    return m_comparisonSampler.Get();
}

ID3D11RasterizerState *DirectionalShadowResources::GetShadowPassRasterizerState() const noexcept
{
    return m_shadowPassRasterizer.Get();
}

float DirectionalShadowResources::GetOrthographicWidth() const noexcept
{
    return m_orthographicWidth;
}

float DirectionalShadowResources::GetOrthographicHeight() const noexcept
{
    return m_orthographicHeight;
}

float DirectionalShadowResources::GetLightEyeDistanceFromFocus() const noexcept
{
    return m_lightEyeDistanceFromFocus;
}

float DirectionalShadowResources::GetNearPlaneDistance() const noexcept
{
    return m_nearPlaneDistance;
}

float DirectionalShadowResources::GetFarPlaneDistance() const noexcept
{
    return m_farPlaneDistance;
}

std::uint32_t DirectionalShadowResources::GetCascadeCount() const noexcept
{
    return m_cascadeCount;
}

std::uint32_t DirectionalShadowResources::GetShadowAtlasSizePixels() const noexcept
{
    return m_shadowAtlasSizePixels;
}

std::uint32_t DirectionalShadowResources::GetCascadeTileSizePixels() const noexcept
{
    if (m_cascadeCount == 0u)
    {
        return m_shadowAtlasSizePixels;
    }
    return m_shadowAtlasSizePixels / 2u;
}

float DirectionalShadowResources::GetCascadeSplitLambda() const noexcept
{
    return m_cascadeSplitLambda;
}

float DirectionalShadowResources::GetEffectiveShadowFarClamp() const noexcept
{
    return m_effectiveShadowFarClamp;
}

void DirectionalShadowResources::CreateConstantBuffers(ID3D11Device *const device)
{
    if (device == nullptr)
    {
        throw std::invalid_argument("DirectionalShadowResources::CreateConstantBuffers requires a device.");
    }

    D3D11_BUFFER_DESC description{};
    description.Usage = D3D11_USAGE_DYNAMIC;
    description.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    description.ByteWidth = static_cast<UINT>(sizeof(ShadowCascadeConstantsGpu));
    D3d11Helpers::ThrowIfFailed(
        device->CreateBuffer(&description, nullptr, m_shadowCascadeConstantBuffer.GetAddressOf()),
        "DirectionalShadowResources failed to create cascade constant buffer."
    );

    description.ByteWidth = static_cast<UINT>(sizeof(ShadowSamplingGpuConstants));
    D3d11Helpers::ThrowIfFailed(
        device->CreateBuffer(&description, nullptr, m_shadowSamplingConstantBuffer.GetAddressOf()),
        "DirectionalShadowResources failed to create sampling constant buffer."
    );
}
