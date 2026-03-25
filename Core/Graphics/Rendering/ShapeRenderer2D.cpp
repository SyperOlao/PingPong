//
// Created by SyperOlao on 18.03.2026.
//

#include "ShapeRenderer2D.h"

#include <array>
#include <cstring>
#include <filesystem>
#include <stdexcept>

#include "../D3d11Helpers.h"
#include "../GraphicsDevice.h"
#include "../ShaderCompiler.h"

void ShapeRenderer2D::Initialize(GraphicsDevice &graphics) {
    m_graphics = &graphics;

    CreateShaders();
    CreateDynamicVertexBuffer();
    CreateDepthStencilState();
}

void ShapeRenderer2D::DrawFilledRect(const RectF &rect, const Color &color) const {
    DrawFilledRect(rect.X, rect.Y, rect.Width, rect.Height, color);
}

void ShapeRenderer2D::DrawFilledRect(
    const float x,
    const float y,
    const float width,
    const float height,
    const Color &color
) const {
    if (!IsInitialized()) {
        throw std::logic_error("ShapeRenderer2D::DrawFilledRect called before Initialize.");
    }

    const float left = ToNdcX(x);
    const float right = ToNdcX(x + width);
    const float top = ToNdcY(y);
    const float bottom = ToNdcY(y + height);

    const std::array<Vertex2D, 6> vertices =
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

    ID3D11DeviceContext *const context = m_graphics->GetImmediateContext();

    D3D11_MAPPED_SUBRESOURCE mappedResource{};
    D3d11Helpers::ThrowIfFailed(
        context->Map(m_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource),
        "ID3D11DeviceContext::Map failed for ShapeRenderer2D vertex buffer."
    );

    std::memcpy(mappedResource.pData, vertices.data(), sizeof(vertices));
    context->Unmap(m_vertexBuffer.Get(), 0);

    BindPipeline();

    constexpr UINT stride = sizeof(Vertex2D);
    constexpr UINT offset = 0;

    ID3D11Buffer *const vertexBuffers[] = {m_vertexBuffer.Get()};
    context->IASetVertexBuffers(0, 1, vertexBuffers, &stride, &offset);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    context->Draw(static_cast<UINT>(vertices.size()), 0);
}

bool ShapeRenderer2D::IsInitialized() const noexcept {
    return m_graphics != nullptr
           && m_vertexShader != nullptr
           && m_pixelShader != nullptr
           && m_inputLayout != nullptr
           && m_vertexBuffer != nullptr
           && m_depthStencilStateNoDepthTest != nullptr;
}

void ShapeRenderer2D::CreateShaders() {
    if (m_graphics == nullptr) {
        throw std::logic_error("ShapeRenderer2D::CreateShaders requires a valid GraphicsDevice.");
    }

    ID3D11Device *const device = m_graphics->GetDevice();

    const auto vertexShaderBlob = ShaderCompiler::CompileFromFile(
        std::filesystem::path("Core/Shaders/Shape2D.hlsl"),
        "VSMain",
        "vs_5_0"
    );

    const auto pixelShaderBlob = ShaderCompiler::CompileFromFile(
        std::filesystem::path("Core/Shaders/Shape2D.hlsl"),
        "PSMain",
        "ps_5_0"
    );

    D3d11Helpers::ThrowIfFailed(
        device->CreateVertexShader(
            vertexShaderBlob->GetBufferPointer(),
            vertexShaderBlob->GetBufferSize(),
            nullptr,
            m_vertexShader.GetAddressOf()
        ),
        "ID3D11Device::CreateVertexShader failed for ShapeRenderer2D."
    );

    D3d11Helpers::ThrowIfFailed(
        device->CreatePixelShader(
            pixelShaderBlob->GetBufferPointer(),
            pixelShaderBlob->GetBufferSize(),
            nullptr,
            m_pixelShader.GetAddressOf()
        ),
        "ID3D11Device::CreatePixelShader failed for ShapeRenderer2D."
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

    D3d11Helpers::ThrowIfFailed(
        device->CreateInputLayout(
            inputLayoutDesc,
            static_cast<UINT>(std::size(inputLayoutDesc)),
            vertexShaderBlob->GetBufferPointer(),
            vertexShaderBlob->GetBufferSize(),
            m_inputLayout.GetAddressOf()
        ),
        "ID3D11Device::CreateInputLayout failed for ShapeRenderer2D."
    );
}

void ShapeRenderer2D::CreateDynamicVertexBuffer() {
    if (m_graphics == nullptr) {
        throw std::logic_error("ShapeRenderer2D::CreateDynamicVertexBuffer requires a valid GraphicsDevice.");
    }

    D3D11_BUFFER_DESC bufferDesc{};
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth = static_cast<UINT>(sizeof(Vertex2D) * 6);
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;

    D3d11Helpers::ThrowIfFailed(
        m_graphics->GetDevice()->CreateBuffer(&bufferDesc, nullptr, m_vertexBuffer.GetAddressOf()),
        "ID3D11Device::CreateBuffer failed for ShapeRenderer2D vertex buffer."
    );
}

void ShapeRenderer2D::CreateDepthStencilState() {
    if (m_graphics == nullptr) {
        throw std::logic_error("ShapeRenderer2D::CreateDepthStencilState requires a valid GraphicsDevice.");
    }

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = FALSE;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthStencilDesc.StencilEnable = FALSE;

    D3d11Helpers::ThrowIfFailed(
        m_graphics->GetDevice()->CreateDepthStencilState(
            &depthStencilDesc,
            m_depthStencilStateNoDepthTest.GetAddressOf()
        ),
        "ShapeRenderer2D failed to create depth stencil state."
    );
}

void ShapeRenderer2D::BindPipeline() const {
    ID3D11DeviceContext *const context = m_graphics->GetImmediateContext();

    context->OMSetDepthStencilState(m_depthStencilStateNoDepthTest.Get(), 0);
    context->IASetInputLayout(m_inputLayout.Get());
    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
}

float ShapeRenderer2D::ToNdcX(const float x) const noexcept {
    return (2.0f * x / static_cast<float>(m_graphics->GetWidth())) - 1.0f;
}

float ShapeRenderer2D::ToNdcY(const float y) const noexcept {
    return 1.0f - (2.0f * y / static_cast<float>(m_graphics->GetHeight()));
}
