//
// Created by SyperOlao on 19.03.2026.
//

#include "PrimitiveRenderer3D.h"
#include <stdexcept>

#include <array>

#include "../ObjectConstants.h"
#include "../ShaderCompiler.h"
#include "../Vertex3D.h"
#include "Core/Graphics/GraphicsDevice.h"

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

namespace {
    constexpr UINT kMaxOrbitVertices = 512;

    void ThrowIfFailed(const HRESULT hr, const char *const message) {
        if (FAILED(hr)) {
            throw std::runtime_error(message);
        }
    }
}

PrimitiveRenderer3D::~PrimitiveRenderer3D() = default;

void PrimitiveRenderer3D::Initialize(GraphicsDevice &graphics) {
    m_graphics = &graphics;

    CreatePrimitives();
    CreateLineResources();
}

void PrimitiveRenderer3D::BeginFrame3D() const {
    if (m_graphics == nullptr) {
        throw std::logic_error("PrimitiveRenderer3D::BeginFrame3D called before Initialize.");
    }

    m_graphics->BindDefaultRenderTargets();
    m_graphics->ClearDefaultDepthStencil();
}

void PrimitiveRenderer3D::DrawBox(
    const Matrix &world,
    const Matrix &view,
    const Matrix &projection,
    const DirectX::SimpleMath::Color &color
) const {
    if (m_graphics == nullptr || m_box == nullptr) {
        throw std::logic_error("PrimitiveRenderer3D::DrawBox called before Initialize.");
    }

    BindTargets();
    m_box->Draw(world, view, projection, color);
}

void PrimitiveRenderer3D::DrawSphere(
    const Matrix &world,
    const Matrix &view,
    const Matrix &projection,
    const DirectX::SimpleMath::Color &color
) const {
    if (m_graphics == nullptr || m_sphere == nullptr) {
        throw std::logic_error("PrimitiveRenderer3D::DrawSphere called before Initialize.");
    }

    BindTargets();
    m_sphere->Draw(world, view, projection, color);
}

void PrimitiveRenderer3D::DrawOrbit(
    const std::vector<Vector3> &points,
    const Matrix &view,
    const Matrix &projection,
    const DirectX::SimpleMath::Color &color
) const {
    if (m_graphics == nullptr) {
        throw std::logic_error("PrimitiveRenderer3D::DrawOrbit called before Initialize.");
    }

    if (points.size() < 2) {
        return;
    }

    if (points.size() > kMaxOrbitVertices) {
        throw std::runtime_error("PrimitiveRenderer3D::DrawOrbit exceeded max orbit vertex count.");
    }

    BindTargets();

    ID3D11DeviceContext *const context = m_graphics->GetImmediateContext();
    if (context == nullptr) {
        throw std::logic_error("PrimitiveRenderer3D::DrawOrbit failed: null context.");
    }

    std::vector<Vertex3D> vertices;
    vertices.reserve(points.size());

    for (const Vector3 &point: points) {
        vertices.emplace_back(point, color);
    }

    D3D11_MAPPED_SUBRESOURCE mappedVertex{};
    ThrowIfFailed(
        context->Map(m_lineVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedVertex),
        "PrimitiveRenderer3D failed to map orbit vertex buffer."
    );

    std::memcpy(mappedVertex.pData, vertices.data(), sizeof(Vertex3D) * vertices.size());
    context->Unmap(m_lineVertexBuffer.Get(), 0);

    ObjectConstants constants{};
    constants.World = Matrix::Identity.Transpose();
    constants.View = view.Transpose();
    constants.Projection = projection.Transpose();

    D3D11_MAPPED_SUBRESOURCE mappedConstants{};
    ThrowIfFailed(
        context->Map(m_lineConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedConstants),
        "PrimitiveRenderer3D failed to map orbit constant buffer."
    );

    std::memcpy(mappedConstants.pData, &constants, sizeof(ObjectConstants));
    context->Unmap(m_lineConstantBuffer.Get(), 0);

    constexpr UINT stride = sizeof(Vertex3D);
    constexpr UINT offset = 0;
    ID3D11Buffer *vertexBuffers[] = {m_lineVertexBuffer.Get()};

    context->IASetInputLayout(m_lineInputLayout.Get());
    context->IASetVertexBuffers(0, 1, vertexBuffers, &stride, &offset);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

    context->VSSetShader(m_lineVertexShader.Get(), nullptr, 0);
    context->VSSetConstantBuffers(0, 1, m_lineConstantBuffer.GetAddressOf());
    context->PSSetShader(m_linePixelShader.Get(), nullptr, 0);

    context->Draw(static_cast<UINT>(vertices.size()), 0);
}

void PrimitiveRenderer3D::CreatePrimitives() {
    if (m_graphics == nullptr) {
        throw std::logic_error("PrimitiveRenderer3D::CreatePrimitives requires initialized graphics.");
    }

    ID3D11DeviceContext *const context = m_graphics->GetImmediateContext();
    if (context == nullptr) {
        throw std::logic_error("PrimitiveRenderer3D::CreatePrimitives failed: null device context.");
    }

    m_box = DirectX::DX11::GeometricPrimitive::CreateCube(context, 1.0f);
    m_sphere = DirectX::DX11::GeometricPrimitive::CreateSphere(context, 1.0f, 32);
}

void PrimitiveRenderer3D::CreateLineResources() {
    if (m_graphics == nullptr) {
        throw std::logic_error("PrimitiveRenderer3D::CreateLineResources requires initialized graphics.");
    }

    ID3D11Device *const device = m_graphics->GetDevice();
    if (device == nullptr) {
        throw std::logic_error("PrimitiveRenderer3D::CreateLineResources failed: null device.");
    }

    const auto vertexShaderBlob = ShaderCompiler::CompileFromFile("Core/Shaders/Basic3D.hlsl", "VSMain", "vs_5_0");
    const auto pixelShaderBlob = ShaderCompiler::CompileFromFile("Core/Shaders/Basic3D.hlsl", "PSMain", "ps_5_0");

    ThrowIfFailed(
        device->CreateVertexShader(
            vertexShaderBlob->GetBufferPointer(),
            vertexShaderBlob->GetBufferSize(),
            nullptr,
            m_lineVertexShader.GetAddressOf()
        ),
        "PrimitiveRenderer3D failed to create orbit vertex shader."
    );

    ThrowIfFailed(
        device->CreatePixelShader(
            pixelShaderBlob->GetBufferPointer(),
            pixelShaderBlob->GetBufferSize(),
            nullptr,
            m_linePixelShader.GetAddressOf()
        ),
        "PrimitiveRenderer3D failed to create orbit pixel shader."
    );

    constexpr D3D11_INPUT_ELEMENT_DESC inputElements[] =
    {
        {
            "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
            D3D11_INPUT_PER_VERTEX_DATA, 0
        },
        {
            "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,
            D3D11_INPUT_PER_VERTEX_DATA, 0
        }
    };

    ThrowIfFailed(
        device->CreateInputLayout(
            inputElements,
            static_cast<UINT>(std::size(inputElements)),
            vertexShaderBlob->GetBufferPointer(),
            vertexShaderBlob->GetBufferSize(),
            m_lineInputLayout.GetAddressOf()
        ),
        "PrimitiveRenderer3D failed to create orbit input layout."
    );

    D3D11_BUFFER_DESC vertexBufferDesc{};
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.ByteWidth = sizeof(Vertex3D) * kMaxOrbitVertices;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    ThrowIfFailed(
        device->CreateBuffer(&vertexBufferDesc, nullptr, m_lineVertexBuffer.GetAddressOf()),
        "PrimitiveRenderer3D failed to create orbit vertex buffer."
    );

    D3D11_BUFFER_DESC constantBufferDesc{};
    constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    constantBufferDesc.ByteWidth = sizeof(ObjectConstants);
    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    ThrowIfFailed(
        device->CreateBuffer(&constantBufferDesc, nullptr, m_lineConstantBuffer.GetAddressOf()),
        "PrimitiveRenderer3D failed to create orbit constant buffer."
    );
}

void PrimitiveRenderer3D::BindTargets() const {
    if (m_graphics == nullptr) {
        throw std::logic_error("PrimitiveRenderer3D::BindTargets requires initialized graphics.");
    }

    m_graphics->BindDefaultRenderTargets();
}
