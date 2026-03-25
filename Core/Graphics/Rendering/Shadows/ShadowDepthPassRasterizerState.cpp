#include "Core/Graphics/Rendering/Shadows/ShadowDepthPassRasterizerState.h"

#include "Core/Graphics/D3d11Helpers.h"

#include <stdexcept>

void ShadowDepthPassRasterizerState::Create(
    ID3D11Device *const device,
    const D3D11_CULL_MODE cullMode,
    const int depthBias,
    const float slopeScaledDepthBias,
    const float depthBiasClamp
)
{
    if (device == nullptr)
    {
        throw std::invalid_argument("ShadowDepthPassRasterizerState::Create requires a device.");
    }

    m_rasterizerState.Reset();

    D3D11_RASTERIZER_DESC rasterizerDescription{};
    rasterizerDescription.FillMode = D3D11_FILL_SOLID;
    rasterizerDescription.CullMode = cullMode;
    rasterizerDescription.FrontCounterClockwise = FALSE;
    rasterizerDescription.DepthBias = depthBias;
    rasterizerDescription.DepthBiasClamp = depthBiasClamp;
    rasterizerDescription.SlopeScaledDepthBias = slopeScaledDepthBias;
    rasterizerDescription.DepthClipEnable = TRUE;
    rasterizerDescription.MultisampleEnable = FALSE;
    rasterizerDescription.AntialiasedLineEnable = FALSE;
    rasterizerDescription.ScissorEnable = FALSE;

    D3d11Helpers::ThrowIfFailed(
        device->CreateRasterizerState(&rasterizerDescription, m_rasterizerState.GetAddressOf()),
        "ShadowDepthPassRasterizerState failed to create rasterizer state."
    );
}

void ShadowDepthPassRasterizerState::Destroy() noexcept
{
    m_rasterizerState.Reset();
}

ID3D11RasterizerState *ShadowDepthPassRasterizerState::Get() const noexcept
{
    return m_rasterizerState.Get();
}
