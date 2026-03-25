//
// Created by SyperOlao on 18.03.2026.
//

#include "GraphicsDevice.h"
#include <array>
#include <stdexcept>

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

void GraphicsDevice::Initialize(HWND__ *const windowHandle, const int width, const int height)
{
    if (windowHandle == nullptr)
    {
        throw std::invalid_argument("GraphicsDevice::Initialize received a null window handle.");
    }

    if (width <= 0 || height <= 0)
    {
        throw std::invalid_argument("GraphicsDevice::Initialize requires positive width and height.");
    }

    m_windowHandle = windowHandle;
    m_width = width;
    m_height = height;

    CreateDeviceAndSwapChain();
    CreateRenderTarget();
    CreateDepthStencil();
    BindBackBufferRenderTargetOnly();
    CreateViewport();
}

void GraphicsDevice::Resize(const int width, const int height)
{
    if (m_swapChain == nullptr)
    {
        throw std::logic_error("GraphicsDevice::Resize called before Initialize.");
    }

    if (width <= 0 || height <= 0)
    {
        return;
    }

    if (width == m_width && height == m_height)
    {
        return;
    }

    m_context->OMSetRenderTargets(0, nullptr, nullptr);
    ReleaseRenderTarget();
    ReleaseDepthStencil();

    ThrowIfFailed(
        m_swapChain->ResizeBuffers(
            0,
            static_cast<UINT>(width),
            static_cast<UINT>(height),
            DXGI_FORMAT_UNKNOWN,
            0
        ),
        "IDXGISwapChain::ResizeBuffers failed."
    );

    m_width = width;
    m_height = height;

    CreateRenderTarget();
    CreateDepthStencil();
    BindBackBufferRenderTargetOnly();
    CreateViewport();
}

void GraphicsDevice::BeginFrame(const Color& clearColor)
{
    if (m_renderTargetView == nullptr)
    {
        throw std::logic_error("GraphicsDevice::BeginFrame called before render target creation.");
    }

    BindBackBufferRenderTargetOnly();

    const auto clear = clearColor.ToArray();
    m_context->ClearRenderTargetView(m_renderTargetView.Get(), clear.data());
}

void GraphicsDevice::BindBackBufferRenderTargetOnly() const
{
    if (m_renderTargetView == nullptr)
    {
        throw std::logic_error("GraphicsDevice::BindBackBufferRenderTargetOnly called before render target creation.");
    }

    ID3D11RenderTargetView* renderTargets[] = {m_renderTargetView.Get()};
    m_context->OMSetRenderTargets(1, renderTargets, nullptr);
}

void GraphicsDevice::BindDefaultRenderTargets() const
{
    if (m_renderTargetView == nullptr)
    {
        throw std::logic_error("GraphicsDevice::BindDefaultRenderTargets called before render target creation.");
    }

    ID3D11RenderTargetView* renderTargets[] = {m_renderTargetView.Get()};
    m_context->OMSetRenderTargets(1, renderTargets, m_depthStencilView.Get());
}

void GraphicsDevice::ClearDefaultDepthStencil() const
{
    if (m_depthStencilView == nullptr)
    {
        return;
    }

    m_context->ClearDepthStencilView(
        m_depthStencilView.Get(),
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
        1.0f,
        0
    );
}

void GraphicsDevice::EndFrame(const bool vSync) const {
    if (m_swapChain == nullptr)
    {
        throw std::logic_error("GraphicsDevice::EndFrame called before Initialize.");
    }

    ThrowIfFailed(
        m_swapChain->Present(vSync ? 1U : 0U, 0U),
        "IDXGISwapChain::Present failed."
    );
}

int GraphicsDevice::GetWidth() const noexcept
{
    return m_width;
}

int GraphicsDevice::GetHeight() const noexcept
{
    return m_height;
}

HWND GraphicsDevice::GetWindowHandle() const noexcept
{
    return m_windowHandle;
}

ID3D11Device* GraphicsDevice::GetDevice() const noexcept
{
    return m_device.Get();
}

ID3D11DeviceContext* GraphicsDevice::GetImmediateContext() const noexcept
{
    return m_context.Get();
}

IDXGISwapChain* GraphicsDevice::GetSwapChain() const noexcept
{
    return m_swapChain.Get();
}

ID3D11RenderTargetView* GraphicsDevice::GetRenderTargetView() const noexcept
{
    return m_renderTargetView.Get();
}

ID3D11DepthStencilView* GraphicsDevice::GetDepthStencilView() const noexcept
{
    return m_depthStencilView.Get();
}

void GraphicsDevice::CreateDeviceAndSwapChain()
{
    DXGI_SWAP_CHAIN_DESC swapChainDesc{};
    swapChainDesc.BufferDesc.Width = static_cast<UINT>(m_width);
    swapChainDesc.BufferDesc.Height = static_cast<UINT>(m_height);
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;

    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 1;
    swapChainDesc.OutputWindow = m_windowHandle;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags = 0;

    UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    constexpr D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };

    D3D_FEATURE_LEVEL createdFeatureLevel{};

    ThrowIfFailed(
        D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            createDeviceFlags,
            featureLevels,
            static_cast<UINT>(std::size(featureLevels)),
            D3D11_SDK_VERSION,
            &swapChainDesc,
            m_swapChain.GetAddressOf(),
            m_device.GetAddressOf(),
            &createdFeatureLevel,
            m_context.GetAddressOf()
        ),
        "D3D11CreateDeviceAndSwapChain failed."
    );

    (void)createdFeatureLevel;
}

void GraphicsDevice::CreateRenderTarget()
{
    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;

    ThrowIfFailed(
        m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())),
        "IDXGISwapChain::GetBuffer failed."
    );

    ThrowIfFailed(
        m_device->CreateRenderTargetView(
            backBuffer.Get(),
            nullptr,
            m_renderTargetView.GetAddressOf()
        ),
        "ID3D11Device::CreateRenderTargetView failed."
    );
}

void GraphicsDevice::CreateDepthStencil()
{
    if (m_device == nullptr)
    {
        throw std::logic_error("GraphicsDevice::CreateDepthStencil called before device creation.");
    }

    if (m_width <= 0 || m_height <= 0)
    {
        throw std::invalid_argument("GraphicsDevice::CreateDepthStencil requires positive dimensions.");
    }

    D3D11_TEXTURE2D_DESC depthDesc{};
    depthDesc.Width = static_cast<UINT>(m_width);
    depthDesc.Height = static_cast<UINT>(m_height);
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.SampleDesc.Quality = 0;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ThrowIfFailed(
        m_device->CreateTexture2D(&depthDesc, nullptr, m_depthTexture.GetAddressOf()),
        "GraphicsDevice failed to create depth texture."
    );

    ThrowIfFailed(
        m_device->CreateDepthStencilView(m_depthTexture.Get(), nullptr, m_depthStencilView.GetAddressOf()),
        "GraphicsDevice failed to create depth stencil view."
    );
}

void GraphicsDevice::CreateViewport() const
{
    D3D11_VIEWPORT viewport{};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(m_width);
    viewport.Height = static_cast<float>(m_height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    m_context->RSSetViewports(1, &viewport);
}

void GraphicsDevice::ReleaseRenderTarget() noexcept
{
    m_renderTargetView.Reset();
}

void GraphicsDevice::ReleaseDepthStencil() noexcept
{
    m_depthStencilView.Reset();
    m_depthTexture.Reset();
}