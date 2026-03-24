//
// Created by SyperOlao on 19.03.2026.
//

#include "PrimitiveRenderer3D.h"
#include <stdexcept>

#include <DirectXColors.h>

#include "Core/Graphics/GraphicsDevice.h"

using DirectX::SimpleMath::Matrix;

namespace
{
    void ThrowIfFailed(const HRESULT hr, const char* const message)
    {
        if (FAILED(hr))
        {
            throw std::runtime_error(message);
        }
    }
}


PrimitiveRenderer3D::~PrimitiveRenderer3D() = default;

void PrimitiveRenderer3D::Initialize(GraphicsDevice& graphics)
{
    m_graphics = &graphics;

    CreatePrimitives();
    EnsureDepthResources();
}

void PrimitiveRenderer3D::BeginFrame3D() const
{
    if (m_graphics == nullptr)
    {
        throw std::logic_error("PrimitiveRenderer3D::BeginFrame3D called before Initialize.");
    }

    EnsureDepthResources();
    BindTargets();

    ID3D11DeviceContext* const context = m_graphics->GetImmediateContext();
    context->ClearDepthStencilView(
        m_depthStencilView.Get(),
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
        1.0f,
        0
    );
}

void PrimitiveRenderer3D::DrawBox(
    const Matrix& world,
    const Matrix& view,
    const Matrix& projection,
    const DirectX::SimpleMath::Color& color
) const
{
    if (m_graphics == nullptr || m_box == nullptr)
    {
        throw std::logic_error("PrimitiveRenderer3D::DrawBox called before Initialize.");
    }

    BindTargets();
    m_box->Draw(world, view, projection, color);
}

void PrimitiveRenderer3D::DrawSphere(
    const Matrix& world,
    const Matrix& view,
    const Matrix& projection,
    const DirectX::SimpleMath::Color& color
) const
{
    if (m_graphics == nullptr || m_sphere == nullptr)
    {
        throw std::logic_error("PrimitiveRenderer3D::DrawSphere called before Initialize.");
    }

    BindTargets();
    m_sphere->Draw(world, view, projection, color);
}

void PrimitiveRenderer3D::CreatePrimitives()
{
    if (m_graphics == nullptr)
    {
        throw std::logic_error("PrimitiveRenderer3D::CreatePrimitives requires initialized graphics.");
    }

    ID3D11DeviceContext* const context = m_graphics->GetImmediateContext();
    if (context == nullptr)
    {
        throw std::logic_error("PrimitiveRenderer3D::CreatePrimitives failed: null device context.");
    }

    m_box = DirectX::DX11::GeometricPrimitive::CreateCube(context, 1.0f);
    m_sphere = DirectX::DX11::GeometricPrimitive::CreateSphere(context, 1.0f, 32);
}

void PrimitiveRenderer3D::EnsureDepthResources() const
{
    if (m_graphics == nullptr)
    {
        throw std::logic_error("PrimitiveRenderer3D::EnsureDepthResources requires initialized graphics.");
    }

    const int width = m_graphics->GetWidth();
    const int height = m_graphics->GetHeight();

    if (width <= 0 || height <= 0)
    {
        return;
    }

    if (m_depthStencilView != nullptr
        && m_cachedDepthWidth == width
        && m_cachedDepthHeight == height)
    {
        return;
    }

    m_depthStencilView.Reset();
    m_depthTexture.Reset();

    D3D11_TEXTURE2D_DESC depthDesc{};
    depthDesc.Width = static_cast<UINT>(width);
    depthDesc.Height = static_cast<UINT>(height);
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.SampleDesc.Quality = 0;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ID3D11Device* const device = m_graphics->GetDevice();
    if (device == nullptr)
    {
        throw std::logic_error("PrimitiveRenderer3D::EnsureDepthResources failed: null device.");
    }

    ThrowIfFailed(
        device->CreateTexture2D(
            &depthDesc,
            nullptr,
            m_depthTexture.GetAddressOf()
        ),
        "PrimitiveRenderer3D failed to create depth texture."
    );

    ThrowIfFailed(
        device->CreateDepthStencilView(
            m_depthTexture.Get(),
            nullptr,
            m_depthStencilView.GetAddressOf()
        ),
        "PrimitiveRenderer3D failed to create depth stencil view."
    );

    m_cachedDepthWidth = width;
    m_cachedDepthHeight = height;
}

void PrimitiveRenderer3D::BindTargets() const
{
    if (m_graphics == nullptr)
    {
        throw std::logic_error("PrimitiveRenderer3D::BindTargets requires initialized graphics.");
    }

    ID3D11DeviceContext* const context = m_graphics->GetImmediateContext();
    ID3D11RenderTargetView* const rtv = m_graphics->GetRenderTargetView();

    if (context == nullptr || rtv == nullptr || m_depthStencilView == nullptr)
    {
        throw std::logic_error("PrimitiveRenderer3D::BindTargets failed: missing D3D targets.");
    }

    ID3D11RenderTargetView* renderTargets[] = { rtv };
    context->OMSetRenderTargets(1, renderTargets, m_depthStencilView.Get());
}