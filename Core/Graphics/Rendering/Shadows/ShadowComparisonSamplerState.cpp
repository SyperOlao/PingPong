#include "Core/Graphics/Rendering/Shadows/ShadowComparisonSamplerState.h"

#include "Core/Graphics/D3d11Helpers.h"

#include <stdexcept>

void ShadowComparisonSamplerState::Create(ID3D11Device *const device)
{
    if (device == nullptr)
    {
        throw std::invalid_argument("ShadowComparisonSamplerState::Create requires a device.");
    }

    m_sampler.Reset();

    D3D11_SAMPLER_DESC samplerDescription{};
    samplerDescription.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    samplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    samplerDescription.MipLODBias = 0.0f;
    samplerDescription.MaxAnisotropy = 1u;
    samplerDescription.ComparisonFunc = D3D11_COMPARISON_GREATER_EQUAL;
    samplerDescription.BorderColor[0] = 1.0f;
    samplerDescription.BorderColor[1] = 1.0f;
    samplerDescription.BorderColor[2] = 1.0f;
    samplerDescription.BorderColor[3] = 1.0f;
    samplerDescription.MinLOD = 0.0f;
    samplerDescription.MaxLOD = D3D11_FLOAT32_MAX;

    D3d11Helpers::ThrowIfFailed(
        device->CreateSamplerState(&samplerDescription, m_sampler.GetAddressOf()),
        "ShadowComparisonSamplerState failed to create sampler state."
    );
}

void ShadowComparisonSamplerState::Destroy() noexcept
{
    m_sampler.Reset();
}

ID3D11SamplerState *ShadowComparisonSamplerState::Get() const noexcept
{
    return m_sampler.Get();
}
