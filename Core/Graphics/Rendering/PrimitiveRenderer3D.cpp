#include "Core/Graphics/Rendering/PrimitiveRenderer3D.h"

#include "Core/Graphics/D3d11Helpers.h"
#include "Core/Graphics/MeshGenerator.h"
#include "Core/Graphics/MeshLitData.h"
#include "Core/Graphics/ObjectConstants.h"
#include "Core/Graphics/Rendering/Lighting/SceneLighting3D.h"
#include "Core/Graphics/Rendering/Lighting/ShaderConstants3D.h"
#include "Core/Graphics/ShaderCompiler.h"
#include "Core/Graphics/Vertex3D.h"
#include "Core/Graphics/GraphicsDevice.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <stdexcept>

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

namespace
{
    constexpr UINT kMaxOrbitVertices = 512;
    constexpr UINT kMaxDebugLineVerticesPerDraw = 65536u;

    static void FillMaterialGpuConstants(
        const RenderMaterialParameters &material,
        MaterialGpuConstants &destination
    )
    {
        const DirectX::XMFLOAT4 &baseColorFloat4 = static_cast<const DirectX::XMFLOAT4 &>(material.BaseColor);
        destination.BaseColor = baseColorFloat4;
        destination.SpecularColorAndPower = DirectX::XMFLOAT4(
            material.SpecularColor.x,
            material.SpecularColor.y,
            material.SpecularColor.z,
            material.SpecularPower
        );
        destination.EmissiveAndAmbient = DirectX::XMFLOAT4(
            material.EmissiveColor.x,
            material.EmissiveColor.y,
            material.EmissiveColor.z,
            material.AmbientFactor
        );
    }

    static void CreateImmutableBuffer(
        ID3D11Device *device,
        const void *initialData,
        UINT byteWidth,
        UINT bindFlags,
        Microsoft::WRL::ComPtr<ID3D11Buffer> &buffer
    )
    {
        D3D11_BUFFER_DESC description{};
        description.Usage = D3D11_USAGE_DEFAULT;
        description.ByteWidth = byteWidth;
        description.BindFlags = bindFlags;

        D3D11_SUBRESOURCE_DATA subresource{};
        subresource.pSysMem = initialData;

        D3d11Helpers::ThrowIfFailed(
            device->CreateBuffer(&description, &subresource, buffer.GetAddressOf()),
            "PrimitiveRenderer3D failed to create immutable buffer."
        );
    }

    static void CreateDynamicConstantBuffer(
        ID3D11Device *device,
        UINT byteWidth,
        Microsoft::WRL::ComPtr<ID3D11Buffer> &buffer
    )
    {
        D3D11_BUFFER_DESC description{};
        description.Usage = D3D11_USAGE_DYNAMIC;
        description.ByteWidth = byteWidth;
        description.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        D3d11Helpers::ThrowIfFailed(
            device->CreateBuffer(&description, nullptr, buffer.GetAddressOf()),
            "PrimitiveRenderer3D failed to create dynamic constant buffer."
        );
    }
}

PrimitiveRenderer3D::~PrimitiveRenderer3D() = default;

void PrimitiveRenderer3D::Initialize(GraphicsDevice &graphics)
{
    m_graphics = &graphics;

    CreatePrimitives();
    CreateLineResources();
    CreateLitResources();
}

void PrimitiveRenderer3D::DrawBox(
    const Matrix &world,
    const Matrix &view,
    const Matrix &projection,
    const DirectX::SimpleMath::Color &color
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
    const Matrix &world,
    const Matrix &view,
    const Matrix &projection,
    const DirectX::SimpleMath::Color &color
) const
{
    if (m_graphics == nullptr || m_sphere == nullptr)
    {
        throw std::logic_error("PrimitiveRenderer3D::DrawSphere called before Initialize.");
    }

    BindTargets();
    m_sphere->Draw(world, view, projection, color);
}

void PrimitiveRenderer3D::DrawBoxLit(
    const Matrix &world,
    const Matrix &view,
    const Matrix &projection,
    const Vector3 &cameraWorldPosition,
    const SceneLightingDescriptor3D &lighting,
    const RenderMaterialParameters &material
) const
{
    if (m_litBoxVertexBuffer == nullptr || m_litBoxIndexBuffer == nullptr)
    {
        throw std::logic_error("PrimitiveRenderer3D::DrawBoxLit called before Initialize.");
    }

    DrawLitMesh(
        m_litBoxVertexBuffer.Get(),
        m_litBoxIndexBuffer.Get(),
        m_litBoxIndexCount,
        world,
        view,
        projection,
        cameraWorldPosition,
        lighting,
        material
    );
}

void PrimitiveRenderer3D::DrawSphereLit(
    const Matrix &world,
    const Matrix &view,
    const Matrix &projection,
    const Vector3 &cameraWorldPosition,
    const SceneLightingDescriptor3D &lighting,
    const RenderMaterialParameters &material
) const
{
    if (m_litSphereVertexBuffer == nullptr || m_litSphereIndexBuffer == nullptr)
    {
        throw std::logic_error("PrimitiveRenderer3D::DrawSphereLit called before Initialize.");
    }

    DrawLitMesh(
        m_litSphereVertexBuffer.Get(),
        m_litSphereIndexBuffer.Get(),
        m_litSphereIndexCount,
        world,
        view,
        projection,
        cameraWorldPosition,
        lighting,
        material
    );
}

void PrimitiveRenderer3D::DrawOrbit(
    const std::vector<Vector3> &points,
    const Matrix &view,
    const Matrix &projection,
    const DirectX::SimpleMath::Color &color
) const
{
    if (m_graphics == nullptr)
    {
        throw std::logic_error("PrimitiveRenderer3D::DrawOrbit called before Initialize.");
    }

    if (points.size() < 2)
    {
        return;
    }

    if (points.size() > kMaxOrbitVertices)
    {
        throw std::runtime_error("PrimitiveRenderer3D::DrawOrbit exceeded max orbit vertex count.");
    }

    BindTargets();

    ID3D11DeviceContext *const context = m_graphics->GetImmediateContext();
    if (context == nullptr)
    {
        throw std::logic_error("PrimitiveRenderer3D::DrawOrbit failed: null context.");
    }

    std::vector<Vertex3D> vertices;
    vertices.reserve(points.size());

    for (const Vector3 &point : points)
    {
        vertices.emplace_back(point, color);
    }

    D3D11_MAPPED_SUBRESOURCE mappedVertex{};
    D3d11Helpers::ThrowIfFailed(
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
    D3d11Helpers::ThrowIfFailed(
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

void PrimitiveRenderer3D::DrawLineList(
    const Vertex3D *vertices,
    const UINT vertexCount,
    const Matrix &view,
    const Matrix &projection
) const
{
    if (m_graphics == nullptr || m_debugLineVertexBuffer == nullptr)
    {
        throw std::logic_error("PrimitiveRenderer3D::DrawLineList called before Initialize.");
    }

    if (vertices == nullptr || vertexCount < 2u)
    {
        return;
    }

    if (vertexCount % 2u != 0u)
    {
        throw std::invalid_argument("PrimitiveRenderer3D::DrawLineList requires an even vertex count.");
    }

    BindTargets();

    ID3D11DeviceContext *const context = m_graphics->GetImmediateContext();
    if (context == nullptr)
    {
        throw std::logic_error("PrimitiveRenderer3D::DrawLineList failed: null context.");
    }

    ObjectConstants constants{};
    constants.World = Matrix::Identity.Transpose();
    constants.View = view.Transpose();
    constants.Projection = projection.Transpose();

    D3D11_MAPPED_SUBRESOURCE mappedConstants{};
    D3d11Helpers::ThrowIfFailed(
        context->Map(m_lineConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedConstants),
        "PrimitiveRenderer3D failed to map line constant buffer for DrawLineList."
    );

    std::memcpy(mappedConstants.pData, &constants, sizeof(ObjectConstants));
    context->Unmap(m_lineConstantBuffer.Get(), 0);

    constexpr UINT stride = sizeof(Vertex3D);
    constexpr UINT offset = 0;
    ID3D11Buffer *vertexBuffers[] = {m_debugLineVertexBuffer.Get()};

    context->IASetInputLayout(m_lineInputLayout.Get());
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    context->VSSetShader(m_lineVertexShader.Get(), nullptr, 0);
    context->VSSetConstantBuffers(0, 1, m_lineConstantBuffer.GetAddressOf());
    context->PSSetShader(m_linePixelShader.Get(), nullptr, 0);

    UINT offsetVertex = 0u;
    while (offsetVertex < vertexCount)
    {
        const UINT remaining = vertexCount - offsetVertex;
        UINT chunkVertexCount = (std::min)(remaining, kMaxDebugLineVerticesPerDraw);
        if (chunkVertexCount % 2u != 0u)
        {
            chunkVertexCount -= 1u;
        }
        if (chunkVertexCount < 2u)
        {
            break;
        }

        D3D11_MAPPED_SUBRESOURCE mappedVertex{};
        D3d11Helpers::ThrowIfFailed(
            context->Map(m_debugLineVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedVertex),
            "PrimitiveRenderer3D failed to map debug line vertex buffer."
        );

        std::memcpy(
            mappedVertex.pData,
            vertices + offsetVertex,
            static_cast<size_t>(sizeof(Vertex3D)) * static_cast<size_t>(chunkVertexCount)
        );
        context->Unmap(m_debugLineVertexBuffer.Get(), 0);

        context->IASetVertexBuffers(0, 1, vertexBuffers, &stride, &offset);
        context->Draw(chunkVertexCount, 0);

        offsetVertex += chunkVertexCount;
    }
}

void PrimitiveRenderer3D::CreatePrimitives()
{
    if (m_graphics == nullptr)
    {
        throw std::logic_error("PrimitiveRenderer3D::CreatePrimitives requires initialized graphics.");
    }

    ID3D11DeviceContext *const context = m_graphics->GetImmediateContext();
    if (context == nullptr)
    {
        throw std::logic_error("PrimitiveRenderer3D::CreatePrimitives failed: null device context.");
    }

    m_box = DirectX::DX11::GeometricPrimitive::CreateCube(context, 1.0f);
    m_sphere = DirectX::DX11::GeometricPrimitive::CreateSphere(context, 1.0f, 32);
}

void PrimitiveRenderer3D::CreateLineResources()
{
    if (m_graphics == nullptr)
    {
        throw std::logic_error("PrimitiveRenderer3D::CreateLineResources requires initialized graphics.");
    }

    ID3D11Device *const device = m_graphics->GetDevice();
    if (device == nullptr)
    {
        throw std::logic_error("PrimitiveRenderer3D::CreateLineResources failed: null device.");
    }

    const auto vertexShaderBlob = ShaderCompiler::CompileFromFile("Core/Shaders/Basic3D.hlsl", "VSMain", "vs_5_0");
    const auto pixelShaderBlob = ShaderCompiler::CompileFromFile("Core/Shaders/Basic3D.hlsl", "PSMain", "ps_5_0");

    D3d11Helpers::ThrowIfFailed(
        device->CreateVertexShader(
            vertexShaderBlob->GetBufferPointer(),
            vertexShaderBlob->GetBufferSize(),
            nullptr,
            m_lineVertexShader.GetAddressOf()
        ),
        "PrimitiveRenderer3D failed to create orbit vertex shader."
    );

    D3d11Helpers::ThrowIfFailed(
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

    D3d11Helpers::ThrowIfFailed(
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

    D3d11Helpers::ThrowIfFailed(
        device->CreateBuffer(&vertexBufferDesc, nullptr, m_lineVertexBuffer.GetAddressOf()),
        "PrimitiveRenderer3D failed to create orbit vertex buffer."
    );

    D3D11_BUFFER_DESC constantBufferDesc{};
    constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    constantBufferDesc.ByteWidth = sizeof(ObjectConstants);
    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3d11Helpers::ThrowIfFailed(
        device->CreateBuffer(&constantBufferDesc, nullptr, m_lineConstantBuffer.GetAddressOf()),
        "PrimitiveRenderer3D failed to create orbit constant buffer."
    );

    D3D11_BUFFER_DESC debugVertexBufferDesc{};
    debugVertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    debugVertexBufferDesc.ByteWidth = sizeof(Vertex3D) * kMaxDebugLineVerticesPerDraw;
    debugVertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    debugVertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3d11Helpers::ThrowIfFailed(
        device->CreateBuffer(&debugVertexBufferDesc, nullptr, m_debugLineVertexBuffer.GetAddressOf()),
        "PrimitiveRenderer3D failed to create debug line vertex buffer."
    );
}

void PrimitiveRenderer3D::CreateLitResources()
{
    if (m_graphics == nullptr)
    {
        throw std::logic_error("PrimitiveRenderer3D::CreateLitResources requires initialized graphics.");
    }

    ID3D11Device *const device = m_graphics->GetDevice();
    if (device == nullptr)
    {
        throw std::logic_error("PrimitiveRenderer3D::CreateLitResources failed: null device.");
    }

    const auto litVertexShaderBlob = ShaderCompiler::CompileFromFile("Core/Shaders/ForwardPhong3D.hlsl", "VSMain", "vs_5_0");
    const auto litPixelShaderBlob = ShaderCompiler::CompileFromFile("Core/Shaders/ForwardPhong3D.hlsl", "PSMain", "ps_5_0");

    D3d11Helpers::ThrowIfFailed(
        device->CreateVertexShader(
            litVertexShaderBlob->GetBufferPointer(),
            litVertexShaderBlob->GetBufferSize(),
            nullptr,
            m_litVertexShader.GetAddressOf()
        ),
        "PrimitiveRenderer3D failed to create lit vertex shader."
    );

    D3d11Helpers::ThrowIfFailed(
        device->CreatePixelShader(
            litPixelShaderBlob->GetBufferPointer(),
            litPixelShaderBlob->GetBufferSize(),
            nullptr,
            m_litPixelShader.GetAddressOf()
        ),
        "PrimitiveRenderer3D failed to create lit pixel shader."
    );

    constexpr UINT kLitVertexStride = sizeof(MeshVertexLit3D);

    constexpr D3D11_INPUT_ELEMENT_DESC litInputElements[] =
    {
        {
            "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
            D3D11_INPUT_PER_VERTEX_DATA, 0
        },
        {
            "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
            D3D11_INPUT_PER_VERTEX_DATA, 0
        },
        {
            "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24,
            D3D11_INPUT_PER_VERTEX_DATA, 0
        }
    };

    D3d11Helpers::ThrowIfFailed(
        device->CreateInputLayout(
            litInputElements,
            static_cast<UINT>(std::size(litInputElements)),
            litVertexShaderBlob->GetBufferPointer(),
            litVertexShaderBlob->GetBufferSize(),
            m_litInputLayout.GetAddressOf()
        ),
        "PrimitiveRenderer3D failed to create lit input layout."
    );

    const MeshLitData boxMesh = MeshGenerator::CreateBoxMeshLit(1.0f, 1.0f, 1.0f);
    const MeshLitData sphereMesh = MeshGenerator::CreateSphereMeshLit(1.0f, 32, 16);

    CreateImmutableBuffer(
        device,
        boxMesh.Vertices.data(),
        static_cast<UINT>(sizeof(MeshVertexLit3D) * boxMesh.Vertices.size()),
        D3D11_BIND_VERTEX_BUFFER,
        m_litBoxVertexBuffer
    );

    CreateImmutableBuffer(
        device,
        boxMesh.Indices.data(),
        static_cast<UINT>(sizeof(std::uint32_t) * boxMesh.Indices.size()),
        D3D11_BIND_INDEX_BUFFER,
        m_litBoxIndexBuffer
    );

    m_litBoxIndexCount = static_cast<UINT>(boxMesh.Indices.size());

    CreateImmutableBuffer(
        device,
        sphereMesh.Vertices.data(),
        static_cast<UINT>(sizeof(MeshVertexLit3D) * sphereMesh.Vertices.size()),
        D3D11_BIND_VERTEX_BUFFER,
        m_litSphereVertexBuffer
    );

    CreateImmutableBuffer(
        device,
        sphereMesh.Indices.data(),
        static_cast<UINT>(sizeof(std::uint32_t) * sphereMesh.Indices.size()),
        D3D11_BIND_INDEX_BUFFER,
        m_litSphereIndexBuffer
    );

    m_litSphereIndexCount = static_cast<UINT>(sphereMesh.Indices.size());

    CreateDynamicConstantBuffer(device, static_cast<UINT>(sizeof(CameraGpuConstants)), m_cameraConstantBuffer);
    CreateDynamicConstantBuffer(device, static_cast<UINT>(sizeof(ObjectGpuConstants)), m_objectConstantBuffer);
    CreateDynamicConstantBuffer(device, static_cast<UINT>(sizeof(MaterialGpuConstants)), m_materialConstantBuffer);
    CreateDynamicConstantBuffer(device, static_cast<UINT>(sizeof(LightsGpuConstants)), m_lightsConstantBuffer);
}

void PrimitiveRenderer3D::BindTargets() const
{
    if (m_graphics == nullptr)
    {
        throw std::logic_error("PrimitiveRenderer3D::BindTargets requires initialized graphics.");
    }

    m_graphics->BindMainRenderTargets();
}

void PrimitiveRenderer3D::DrawLitMesh(
    ID3D11Buffer *vertexBuffer,
    ID3D11Buffer *indexBuffer,
    const UINT indexCount,
    const Matrix &world,
    const Matrix &view,
    const Matrix &projection,
    const Vector3 &cameraWorldPosition,
    const SceneLightingDescriptor3D &lighting,
    const RenderMaterialParameters &material
) const
{
    if (m_graphics == nullptr)
    {
        throw std::logic_error("PrimitiveRenderer3D::DrawLitMesh requires initialized graphics.");
    }

    BindTargets();

    ID3D11DeviceContext *const context = m_graphics->GetImmediateContext();
    if (context == nullptr)
    {
        throw std::logic_error("PrimitiveRenderer3D::DrawLitMesh failed: null context.");
    }

    const Matrix viewProjection = view * projection;

    CameraGpuConstants cameraConstants{};
    cameraConstants.View = view.Transpose();
    cameraConstants.Projection = projection.Transpose();
    cameraConstants.ViewProjection = viewProjection.Transpose();
    cameraConstants.WorldPosition = DirectX::XMFLOAT4(
        cameraWorldPosition.x,
        cameraWorldPosition.y,
        cameraWorldPosition.z,
        1.0f
    );

    const Matrix worldInverse = world.Invert();
    const Matrix worldInverseTranspose = worldInverse.Transpose();

    ObjectGpuConstants objectConstants{};
    objectConstants.World = world.Transpose();
    objectConstants.WorldInverseTranspose = worldInverseTranspose.Transpose();

    MaterialGpuConstants materialConstants{};
    FillMaterialGpuConstants(material, materialConstants);

    LightsGpuConstants lightsConstants{};
    FillLightsGpuConstants(lighting, lightsConstants);

    D3D11_MAPPED_SUBRESOURCE mappedCamera{};
    D3d11Helpers::ThrowIfFailed(
        context->Map(m_cameraConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedCamera),
        "PrimitiveRenderer3D failed to map camera constant buffer."
    );
    std::memcpy(mappedCamera.pData, &cameraConstants, sizeof(CameraGpuConstants));
    context->Unmap(m_cameraConstantBuffer.Get(), 0);

    D3D11_MAPPED_SUBRESOURCE mappedObject{};
    D3d11Helpers::ThrowIfFailed(
        context->Map(m_objectConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedObject),
        "PrimitiveRenderer3D failed to map object constant buffer."
    );
    std::memcpy(mappedObject.pData, &objectConstants, sizeof(ObjectGpuConstants));
    context->Unmap(m_objectConstantBuffer.Get(), 0);

    D3D11_MAPPED_SUBRESOURCE mappedMaterial{};
    D3d11Helpers::ThrowIfFailed(
        context->Map(m_materialConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedMaterial),
        "PrimitiveRenderer3D failed to map material constant buffer."
    );
    std::memcpy(mappedMaterial.pData, &materialConstants, sizeof(MaterialGpuConstants));
    context->Unmap(m_materialConstantBuffer.Get(), 0);

    D3D11_MAPPED_SUBRESOURCE mappedLights{};
    D3d11Helpers::ThrowIfFailed(
        context->Map(m_lightsConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedLights),
        "PrimitiveRenderer3D failed to map lights constant buffer."
    );
    std::memcpy(mappedLights.pData, &lightsConstants, sizeof(LightsGpuConstants));
    context->Unmap(m_lightsConstantBuffer.Get(), 0);

    constexpr UINT kLitVertexStride = sizeof(MeshVertexLit3D);
    constexpr UINT kVertexOffset = 0;
    ID3D11Buffer *vertexBuffers[] = {vertexBuffer};

    context->IASetInputLayout(m_litInputLayout.Get());
    context->IASetVertexBuffers(0, 1, vertexBuffers, &kLitVertexStride, &kVertexOffset);
    context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D11Buffer *const constantBuffers[] =
    {
        m_cameraConstantBuffer.Get(),
        m_objectConstantBuffer.Get(),
        m_materialConstantBuffer.Get(),
        m_lightsConstantBuffer.Get()
    };

    context->VSSetShader(m_litVertexShader.Get(), nullptr, 0);
    context->VSSetConstantBuffers(0, 4, constantBuffers);

    context->PSSetShader(m_litPixelShader.Get(), nullptr, 0);
    context->PSSetConstantBuffers(0, 4, constantBuffers);

    context->DrawIndexed(indexCount, 0, 0);
}
