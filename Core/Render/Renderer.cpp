//
// Created by SyperOlao on 17.03.2026.
//

#include "Renderer.h"

#include <cstring>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <stdexcept>
#include <vector>
#include <format>

#include "Vertex.h"

namespace {
    void ThrowIfFailed(const HRESULT hr, const char *message) {
        if (FAILED(hr)) {
            throw std::runtime_error(message);
        }
    }
}

void Renderer::Initialize(HWND__ *const hWnd, const int width, const int height) {
    m_width = width;
    m_height = height;

    CreateDeviceAndSwapChain(hWnd);
    CreateRenderTarget();
    CreateViewport();
    LoadShaders();
    CreateDynamicVertexBuffer();
    SetupPipeline();
}

void Renderer::BeginFrame(const Color &clearColor) const {
    const std::array color = {clearColor.r, clearColor.g, clearColor.b, clearColor.a};

    m_context->ClearRenderTargetView(m_renderTargetView.Get(), color.data());
}

void Renderer::EndFrame() const {
    ThrowIfFailed(m_swapChain->Present(1, 0), "IDXGISwapChain::Present failed");
}

void Renderer::DrawRectangle(const float x, const float y, const float width, const float height,
                             const Color &color) const {
    const float left = ToNdcX(x);
    const float right = ToNdcX(x + width);
    const float top = ToNdcY(y);
    const float bottom = ToNdcY(y + height);

    const std::array<Vertex, 6> vertices =
    {
        {
            {left, top, 0.0f, color.r, color.g, color.b, color.a},
            {right, bottom, 0.0f, color.r, color.g, color.b, color.a},
            {left, bottom, 0.0f, color.r, color.g, color.b, color.a},

            {left, top, 0.0f, color.r, color.g, color.b, color.a},
            {right, top, 0.0f, color.r, color.g, color.b, color.a},
            {right, bottom, 0.0f, color.r, color.g, color.b, color.a}
        }
    };

    D3D11_MAPPED_SUBRESOURCE mappedResource{};
    ThrowIfFailed(
        m_context->Map(m_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource),
        "ID3D11DeviceContext::Map failed"
    );

    std::memcpy(mappedResource.pData, vertices.data(), sizeof(vertices));
    m_context->Unmap(m_vertexBuffer.Get(), 0);

    constexpr UINT stride = sizeof(Vertex);
    constexpr UINT offset = 0;

    ID3D11Buffer *buffers[] = {m_vertexBuffer.Get()};
    m_context->IASetVertexBuffers(0, 1, buffers, &stride, &offset);
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_context->Draw((vertices.size()), 0);
}

int Renderer::GetWidth() const noexcept {
    return m_width;
}

int Renderer::GetHeight() const noexcept {
    return m_height;
}

void Renderer::CreateDeviceAndSwapChain(HWND__ *const hWnd) {
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
    swapChainDesc.OutputWindow = hWnd;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags = 0;

    UINT createDeviceFlags = 0;

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
            std::size(featureLevels),
            D3D11_SDK_VERSION,
            &swapChainDesc,
            m_swapChain.GetAddressOf(),
            m_device.GetAddressOf(),
            &createdFeatureLevel,
            m_context.GetAddressOf()
        ),
        "D3D11CreateDeviceAndSwapChain failed"
    );
}

void Renderer::CreateRenderTarget() {
    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;

    ThrowIfFailed(
        m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())),
        "IDXGISwapChain::GetBuffer failed"
    );

    ThrowIfFailed(
        m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf()),
        "ID3D11Device::CreateRenderTargetView failed"
    );

    ID3D11RenderTargetView *renderTargets[] = {m_renderTargetView.Get()};
    m_context->OMSetRenderTargets(1, renderTargets, nullptr);
}

void Renderer::CreateViewport() const {
    D3D11_VIEWPORT viewport{};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(m_width);
    viewport.Height = static_cast<float>(m_height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    m_context->RSSetViewports(1, &viewport);
}

void Renderer::LoadShaders()
{
    const auto vertexShaderBlob = CompileShader(L"Core/Shaders/BasicVS.hlsl", "VSMain", "vs_5_0");
    const auto pixelShaderBlob = CompileShader(L"Core/Shaders/BasicPS.hlsl", "PSMain", "ps_5_0");

    ThrowIfFailed(
        m_device->CreateVertexShader(
            vertexShaderBlob->GetBufferPointer(),
            vertexShaderBlob->GetBufferSize(),
            nullptr,
            m_vertexShader.GetAddressOf()
        ),
        "CreateVertexShader failed"
    );

    ThrowIfFailed(
        m_device->CreatePixelShader(
            pixelShaderBlob->GetBufferPointer(),
            pixelShaderBlob->GetBufferSize(),
            nullptr,
            m_pixelShader.GetAddressOf()
        ),
        "CreatePixelShader failed"
    );

    constexpr D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
    {
        {
            "POSITION",
            0,
            DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            0,
            D3D11_INPUT_PER_VERTEX_DATA,
            0
        },
        {
            "COLOR",
            0,
            DXGI_FORMAT_R32G32B32A32_FLOAT,
            0,
            12,
            D3D11_INPUT_PER_VERTEX_DATA,
            0
        }
    };

    ThrowIfFailed(
        m_device->CreateInputLayout(
            inputLayoutDesc,
            std::size(inputLayoutDesc),
            vertexShaderBlob->GetBufferPointer(),
            vertexShaderBlob->GetBufferSize(),
            m_inputLayout.GetAddressOf()
        ),
        "CreateInputLayout failed"
    );
}
void Renderer::CreateDynamicVertexBuffer()
{
    D3D11_BUFFER_DESC bufferDesc {};
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * 6);
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;

    ThrowIfFailed(
        m_device->CreateBuffer(&bufferDesc, nullptr, m_vertexBuffer.GetAddressOf()),
        "CreateBuffer for vertex buffer failed"
    );
}

void Renderer::SetupPipeline() const {
    m_context->IASetInputLayout(m_inputLayout.Get());
    m_context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    m_context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
}

float Renderer::ToNdcX(float x) const noexcept
{
    return (2.0f * x / static_cast<float>(m_width)) - 1.0f;
}

float Renderer::ToNdcY(float y) const noexcept
{
    return 1.0f - (2.0f * y / static_cast<float>(m_height));
}

Microsoft::WRL::ComPtr<ID3DBlob> Renderer::CompileShader(
    const std::wstring_view filePath,
    const std::string_view entryPoint,
    const std::string_view profile
)
{
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined(_DEBUG)
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

    const HRESULT hr = D3DCompileFromFile(
        filePath.data(),
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint.data(),
        profile.data(),
        flags,
        0,
        shaderBlob.GetAddressOf(),
        errorBlob.GetAddressOf()
    );

    if (FAILED(hr))
    {
        std::string errorText = "Shader compilation failed";

        if (errorBlob != nullptr)
        {
            const auto* buffer = static_cast<const char*>(errorBlob->GetBufferPointer());
            errorText = std::string(buffer, buffer + errorBlob->GetBufferSize());
        }

        throw std::runtime_error(errorText);
    }

    return shaderBlob;
}