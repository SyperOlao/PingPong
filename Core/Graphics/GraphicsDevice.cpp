#include "Core/Graphics/GraphicsDevice.h"

#include "Core/Graphics/D3d11Helpers.h"

#include <array>
#include <stdexcept>

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
    CreateMainPassDepthStencilState();
    CreateRenderTarget();
    CreateDepthStencil();
    BindMainRenderTargets();
    SetMainViewport();
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

    D3d11Helpers::ThrowIfFailed(
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
    BindMainRenderTargets();
    SetMainViewport();
}

void GraphicsDevice::BeginFrame(const Color &clearColor)
{
    if (m_mainRenderTargetView == nullptr)
    {
        throw std::logic_error("GraphicsDevice::BeginFrame called before render target creation.");
    }

    BindMainRenderTargets();
    ClearMainColor(clearColor);
    ClearMainDepthStencil();
    SetMainViewport();
    if (m_mainPassDepthStencilState != nullptr)
    {
        m_context->OMSetDepthStencilState(m_mainPassDepthStencilState.Get(), 0);
    }
}

void GraphicsDevice::BindMainRenderTargets() const
{
    if (m_mainRenderTargetView == nullptr)
    {
        throw std::logic_error("GraphicsDevice::BindMainRenderTargets called before render target creation.");
    }

    ID3D11RenderTargetView *const renderTargets[] = {m_mainRenderTargetView.Get()};
    m_context->OMSetRenderTargets(1, renderTargets, m_mainDepthStencilView.Get());
}

void GraphicsDevice::BindSingleRenderTarget(ID3D11RenderTargetView *const renderTargetView) const
{
    if (renderTargetView == nullptr)
    {
        throw std::invalid_argument("GraphicsDevice::BindSingleRenderTarget requires a render target view.");
    }

    m_context->OMSetRenderTargets(1, &renderTargetView, nullptr);
}

void GraphicsDevice::BindRenderTargetsAndDepth(
    ID3D11RenderTargetView *const *const renderTargetViews,
    const UINT renderTargetCount,
    ID3D11DepthStencilView *const depthStencilView
) const
{
    if (renderTargetCount > 0u && renderTargetViews == nullptr)
    {
        throw std::invalid_argument("GraphicsDevice::BindRenderTargetsAndDepth requires render target views.");
    }

    if (renderTargetCount > D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT)
    {
        throw std::invalid_argument("GraphicsDevice::BindRenderTargetsAndDepth render target count is too large.");
    }

    m_context->OMSetRenderTargets(renderTargetCount, renderTargetViews, depthStencilView);
}

void GraphicsDevice::ClearRenderTargetView(ID3D11RenderTargetView *const renderTargetView, const Color &clearColor) const
{
    if (renderTargetView == nullptr)
    {
        throw std::invalid_argument("GraphicsDevice::ClearRenderTargetView requires a render target view.");
    }

    const auto clear = clearColor.ToArray();
    m_context->ClearRenderTargetView(renderTargetView, clear.data());
}

void GraphicsDevice::ClearMainColor(const Color &clearColor) const
{
    if (m_mainRenderTargetView == nullptr)
    {
        throw std::logic_error("GraphicsDevice::ClearMainColor called before render target creation.");
    }

    const auto clear = clearColor.ToArray();
    m_context->ClearRenderTargetView(m_mainRenderTargetView.Get(), clear.data());
}

void GraphicsDevice::ClearMainDepthStencil(const float depthClearValue, const uint8_t stencilClear) const
{
    if (m_mainDepthStencilView == nullptr)
    {
        return;
    }

    m_context->ClearDepthStencilView(
        m_mainDepthStencilView.Get(),
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
        depthClearValue,
        stencilClear
    );
}

void GraphicsDevice::SetMainViewport() const
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

void GraphicsDevice::BindDepthOnlyRenderTarget(ID3D11DepthStencilView *const depthStencilView) const
{
    if (depthStencilView == nullptr)
    {
        throw std::invalid_argument("GraphicsDevice::BindDepthOnlyRenderTarget requires a depth stencil view.");
    }

    m_context->OMSetRenderTargets(0, nullptr, depthStencilView);
}

void GraphicsDevice::SetViewportPixels(
    const float width,
    const float height,
    const float topLeftX,
    const float topLeftY
) const
{
    D3D11_VIEWPORT viewport{};
    viewport.TopLeftX = topLeftX;
    viewport.TopLeftY = topLeftY;
    viewport.Width = width;
    viewport.Height = height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    m_context->RSSetViewports(1, &viewport);
}

void GraphicsDevice::ClearDepthStencilView(
    ID3D11DepthStencilView *const depthStencilView,
    const float depthClearValue,
    const uint8_t stencilClear
) const
{
    if (depthStencilView == nullptr)
    {
        throw std::invalid_argument("GraphicsDevice::ClearDepthStencilView requires a depth stencil view.");
    }

    m_context->ClearDepthStencilView(
        depthStencilView,
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
        depthClearValue,
        stencilClear
    );
}

void GraphicsDevice::EndFrame(const bool vSync) const
{
    if (m_swapChain == nullptr)
    {
        throw std::logic_error("GraphicsDevice::EndFrame called before Initialize.");
    }

    D3d11Helpers::ThrowIfFailed(
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

ID3D11Device *GraphicsDevice::GetDevice() const noexcept
{
    return m_device.Get();
}

ID3D11DeviceContext *GraphicsDevice::GetImmediateContext() const noexcept
{
    return m_context.Get();
}

D3D_FEATURE_LEVEL GraphicsDevice::GetFeatureLevel() const noexcept
{
    return m_featureLevel;
}

IDXGISwapChain *GraphicsDevice::GetSwapChain() const noexcept
{
    return m_swapChain.Get();
}

ID3D11RenderTargetView *GraphicsDevice::GetMainRenderTargetView() const noexcept
{
    return m_mainRenderTargetView.Get();
}

ID3D11DepthStencilView *GraphicsDevice::GetMainDepthStencilView() const noexcept
{
    return m_mainDepthStencilView.Get();
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
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };

    D3d11Helpers::ThrowIfFailed(
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
            &m_featureLevel,
            m_context.GetAddressOf()
        ),
        "D3D11CreateDeviceAndSwapChain failed."
    );
}

void GraphicsDevice::CreateRenderTarget()
{
    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;

    D3d11Helpers::ThrowIfFailed(
        m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())),
        "IDXGISwapChain::GetBuffer failed."
    );

    D3d11Helpers::ThrowIfFailed(
        m_device->CreateRenderTargetView(
            backBuffer.Get(),
            nullptr,
            m_mainRenderTargetView.GetAddressOf()
        ),
        "ID3D11Device::CreateRenderTargetView failed."
    );
}

void GraphicsDevice::CreateMainPassDepthStencilState()
{
    if (m_device == nullptr)
    {
        throw std::logic_error("GraphicsDevice::CreateMainPassDepthStencilState called before device creation.");
    }

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = TRUE;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthStencilDesc.StencilEnable = FALSE;

    D3d11Helpers::ThrowIfFailed(
        m_device->CreateDepthStencilState(&depthStencilDesc, m_mainPassDepthStencilState.GetAddressOf()),
        "GraphicsDevice failed to create main-pass depth stencil state."
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

    D3d11Helpers::ThrowIfFailed(
        m_device->CreateTexture2D(&depthDesc, nullptr, m_mainDepthTexture.GetAddressOf()),
        "GraphicsDevice failed to create depth texture."
    );

    D3d11Helpers::ThrowIfFailed(
        m_device->CreateDepthStencilView(m_mainDepthTexture.Get(), nullptr, m_mainDepthStencilView.GetAddressOf()),
        "GraphicsDevice failed to create depth stencil view."
    );
}

void GraphicsDevice::ReleaseRenderTarget() noexcept
{
    m_mainRenderTargetView.Reset();
}

void GraphicsDevice::ReleaseDepthStencil() noexcept
{
    m_mainDepthStencilView.Reset();
    m_mainDepthTexture.Reset();
}
