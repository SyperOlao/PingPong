#include "Core/Graphics/ModelRenderer.h"

#include "Core/Graphics/D3d11Helpers.h"
#include "Core/Graphics/GraphicsDevice.h"
#include "Core/Graphics/ModelAsset.h"
#include "Core/Graphics/Rendering/ForwardPhongGpuUpload.h"
#include "Core/Graphics/Rendering/Lighting/ForwardPhongMaterialGpu.h"
#include "Core/Graphics/Rendering/Lighting/SceneLighting3D.h"
#include "Core/Graphics/Rendering/Lighting/ShaderConstants3D.h"
#include "Core/Graphics/Rendering/RenderContext.h"
#include "Core/Graphics/Rendering/Shadows/ShadowShaderConstants3D.h"
#include "Core/Graphics/ShaderCompiler.h"
#include "Core/Assets/AssetPathResolver.h"

#include <directxtk/Effects.h>
#include <directxtk/Model.h>
#include <directxtk/VertexTypes.h>

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <stdexcept>
#include <string>

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

namespace
{
    constexpr D3D11_INPUT_ELEMENT_DESC kForwardPhongModelInputElements[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    void CreateForwardPhongDefaultWhiteDiffuseTexture(
        ID3D11Device *const device,
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> &outShaderResourceView
    )
    {
        if (device == nullptr)
        {
            throw std::invalid_argument("CreateForwardPhongDefaultWhiteDiffuseTexture requires a device.");
        }

        const std::uint32_t rgbaPixel = 0xffffffffu;
        D3D11_SUBRESOURCE_DATA initialData{};
        initialData.pSysMem = &rgbaPixel;
        initialData.SysMemPitch = sizeof(std::uint32_t);

        D3D11_TEXTURE2D_DESC textureDescription{};
        textureDescription.Width = 1u;
        textureDescription.Height = 1u;
        textureDescription.MipLevels = 1u;
        textureDescription.ArraySize = 1u;
        textureDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDescription.SampleDesc.Count = 1u;
        textureDescription.Usage = D3D11_USAGE_IMMUTABLE;
        textureDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2D{};
        D3d11Helpers::ThrowIfFailed(
            device->CreateTexture2D(&textureDescription, &initialData, texture2D.GetAddressOf()),
            "ModelRenderer failed to create default white diffuse texture."
        );

        D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDescription{};
        shaderResourceViewDescription.Format = textureDescription.Format;
        shaderResourceViewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        shaderResourceViewDescription.Texture2D.MipLevels = 1u;
        shaderResourceViewDescription.Texture2D.MostDetailedMip = 0u;

        D3d11Helpers::ThrowIfFailed(
            device->CreateShaderResourceView(
                texture2D.Get(),
                &shaderResourceViewDescription,
                outShaderResourceView.GetAddressOf()
            ),
            "ModelRenderer failed to create default white diffuse shader resource view."
        );
    }

    void CreateDynamicConstantBuffer(
        ID3D11Device *device,
        const UINT byteWidth,
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
            "ModelRenderer failed to create dynamic constant buffer."
        );
    }

    void UploadConstantBuffer(
        ID3D11DeviceContext *const deviceContext,
        ID3D11Buffer *const constantBuffer,
        const void *const data,
        const std::size_t byteCount,
        const char *const errorMessage
    )
    {
        D3D11_MAPPED_SUBRESOURCE mapped{};
        D3d11Helpers::ThrowIfFailed(
            deviceContext->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped),
            errorMessage
        );
        std::memcpy(mapped.pData, data, byteCount);
        deviceContext->Unmap(constantBuffer, 0);
    }

    ID3D11RasterizerState *ResolveRasterizerForMesh(
        DirectX::CommonStates &commonStates,
        const DirectX::ModelMesh &,
        const RenderMaterialParameters &material
    ) noexcept
    {
        if (material.Wireframe)
        {
            return commonStates.Wireframe();
        }

        return commonStates.CullNone();
    }
}

static void ApplyTintToEffect(
    DirectX::IEffect *effectTarget,
    const DirectX::XMVECTOR &colorAndAlpha,
    const RenderMaterialParameters &material
)
{
    if (DirectX::BasicEffect *const effectBasic = dynamic_cast<DirectX::BasicEffect *>(effectTarget))
    {
        effectBasic->SetColorAndAlpha(colorAndAlpha);
        effectBasic->SetLightingEnabled(material.ReceiveLighting);
        if (material.ReceiveLighting)
        {
            effectBasic->EnableDefaultLighting();
            effectBasic->SetPerPixelLighting(true);
            effectBasic->SetAmbientLightColor(DirectX::SimpleMath::Vector3(
                material.AmbientFactor,
                material.AmbientFactor,
                material.AmbientFactor
            ));
            effectBasic->SetSpecularColor(DirectX::SimpleMath::Vector3(
                material.SpecularColor.x,
                material.SpecularColor.y,
                material.SpecularColor.z
            ));
            effectBasic->SetSpecularPower(material.SpecularPower);
            effectBasic->SetEmissiveColor(DirectX::SimpleMath::Vector3(
                material.EmissiveColor.x,
                material.EmissiveColor.y,
                material.EmissiveColor.z
            ));
        }
        return;
    }

    if (DirectX::AlphaTestEffect *const effectAlphaTest = dynamic_cast<DirectX::AlphaTestEffect *>(effectTarget))
    {
        effectAlphaTest->SetColorAndAlpha(colorAndAlpha);
        return;
    }

    if (DirectX::DualTextureEffect *const effectDualTexture = dynamic_cast<DirectX::DualTextureEffect *>(effectTarget))
    {
        effectDualTexture->SetDiffuseColor(colorAndAlpha);
        effectDualTexture->SetAlpha(DirectX::XMVectorGetW(colorAndAlpha));
        return;
    }

    if (DirectX::EnvironmentMapEffect *const effectEnvironmentMap = dynamic_cast<DirectX::EnvironmentMapEffect *>(effectTarget))
    {
        effectEnvironmentMap->SetColorAndAlpha(colorAndAlpha);
        return;
    }

    if (DirectX::SkinnedEffect *const effectSkinned = dynamic_cast<DirectX::SkinnedEffect *>(effectTarget))
    {
        effectSkinned->SetColorAndAlpha(colorAndAlpha);
        effectSkinned->EnableDefaultLighting();
        effectSkinned->SetPerPixelLighting(material.ReceiveLighting);
        effectSkinned->SetSpecularColor(DirectX::SimpleMath::Vector3(
            material.SpecularColor.x,
            material.SpecularColor.y,
            material.SpecularColor.z
        ));
        effectSkinned->SetSpecularPower(material.SpecularPower);
        return;
    }

    if (DirectX::NormalMapEffect *const effectNormalMap = dynamic_cast<DirectX::NormalMapEffect *>(effectTarget))
    {
        effectNormalMap->SetColorAndAlpha(colorAndAlpha);
        effectNormalMap->SetSpecularColor(DirectX::SimpleMath::Vector3(
            material.SpecularColor.x,
            material.SpecularColor.y,
            material.SpecularColor.z
        ));
        effectNormalMap->SetSpecularPower(material.SpecularPower);
        return;
    }
}

void ModelRenderer::SetShadowBindingHost(RenderContext *const host) noexcept
{
    m_shadowBindingHost = host;
}

void ModelRenderer::Initialize(GraphicsDevice &graphics)
{
    m_graphics = &graphics;
    m_commonStates = std::make_unique<DirectX::CommonStates>(graphics.GetDevice());
    CreateForwardPhongResources(graphics);
    CreateDeferredGeometryResources(graphics);
    CreateShadowDepthPassResources(graphics);
}

void ModelRenderer::CreateDeferredGeometryResources(GraphicsDevice &graphics)
{
    ID3D11Device *const device = graphics.GetDevice();
    if (device == nullptr)
    {
        throw std::logic_error("ModelRenderer::CreateDeferredGeometryResources failed: null device.");
    }

    const auto modelVertexShaderBlob = ShaderCompiler::CompileFromFile(
        "Core/Shaders/DeferredGeometry3D.hlsl",
        "VSMainTextured",
        "vs_5_0"
    );
    const auto pixelShaderBlob = ShaderCompiler::CompileFromFile(
        "Core/Shaders/DeferredGeometry3D.hlsl",
        "PSMainTextured",
        "ps_5_0"
    );

    D3d11Helpers::ThrowIfFailed(
        device->CreateVertexShader(
            modelVertexShaderBlob->GetBufferPointer(),
            modelVertexShaderBlob->GetBufferSize(),
            nullptr,
            m_deferredGeometryModelVertexShader.GetAddressOf()
        ),
        "ModelRenderer failed to create deferred geometry model vertex shader."
    );

    D3d11Helpers::ThrowIfFailed(
        device->CreatePixelShader(
            pixelShaderBlob->GetBufferPointer(),
            pixelShaderBlob->GetBufferSize(),
            nullptr,
            m_deferredGeometryPixelShader.GetAddressOf()
        ),
        "ModelRenderer failed to create deferred geometry pixel shader."
    );

    D3d11Helpers::ThrowIfFailed(
        device->CreateInputLayout(
            kForwardPhongModelInputElements,
            static_cast<UINT>(std::size(kForwardPhongModelInputElements)),
            modelVertexShaderBlob->GetBufferPointer(),
            modelVertexShaderBlob->GetBufferSize(),
            m_deferredGeometryModelInputLayout.GetAddressOf()
        ),
        "ModelRenderer failed to create deferred geometry model input layout."
    );
}

void ModelRenderer::CreateShadowDepthPassResources(GraphicsDevice &graphics)
{
    ID3D11Device *const device = graphics.GetDevice();
    if (device == nullptr)
    {
        throw std::logic_error("ModelRenderer::CreateShadowDepthPassResources failed: null device.");
    }

    const auto shadowVertexShaderBlob = ShaderCompiler::CompileFromFile(
        "Core/Shaders/ShadowDepth3D.hlsl",
        "VSMain",
        "vs_5_0"
    );

    D3d11Helpers::ThrowIfFailed(
        device->CreateVertexShader(
            shadowVertexShaderBlob->GetBufferPointer(),
            shadowVertexShaderBlob->GetBufferSize(),
            nullptr,
            m_shadowDepthVertexShader.GetAddressOf()
        ),
        "ModelRenderer failed to create shadow depth vertex shader."
    );

    constexpr D3D11_INPUT_ELEMENT_DESC shadowInputElements[] =
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
            "NORMAL",
            0,
            DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            12,
            D3D11_INPUT_PER_VERTEX_DATA,
            0
        },
        {
            "TEXCOORD",
            0,
            DXGI_FORMAT_R32G32_FLOAT,
            0,
            24,
            D3D11_INPUT_PER_VERTEX_DATA,
            0
        }
    };

    D3d11Helpers::ThrowIfFailed(
        device->CreateInputLayout(
            shadowInputElements,
            static_cast<UINT>(std::size(shadowInputElements)),
            shadowVertexShaderBlob->GetBufferPointer(),
            shadowVertexShaderBlob->GetBufferSize(),
            m_shadowDepthInputLayout.GetAddressOf()
        ),
        "ModelRenderer failed to create shadow depth input layout."
    );

    CreateDynamicConstantBuffer(device, static_cast<UINT>(sizeof(ShadowDepthObjectGpuConstants)), m_shadowDepthConstantBuffer);
}

void ModelRenderer::CreateForwardPhongResources(GraphicsDevice &graphics)
{
    ID3D11Device *const device = graphics.GetDevice();
    if (device == nullptr)
    {
        throw std::logic_error("ModelRenderer::CreateForwardPhongResources failed: null device.");
    }

    const auto modelVertexShaderBlob = ShaderCompiler::CompileFromFile(
        "Core/Shaders/ForwardPhong3D.hlsl",
        "VSMainNoVertexColor",
        "vs_5_0"
    );
    const auto pixelShaderBlob = ShaderCompiler::CompileFromFile(
        "Core/Shaders/ForwardPhong3D.hlsl",
        "PSMain",
        "ps_5_0"
    );

    D3d11Helpers::ThrowIfFailed(
        device->CreateVertexShader(
            modelVertexShaderBlob->GetBufferPointer(),
            modelVertexShaderBlob->GetBufferSize(),
            nullptr,
            m_forwardPhongModelVertexShader.GetAddressOf()
        ),
        "ModelRenderer failed to create forward Phong model vertex shader."
    );

    D3d11Helpers::ThrowIfFailed(
        device->CreatePixelShader(
            pixelShaderBlob->GetBufferPointer(),
            pixelShaderBlob->GetBufferSize(),
            nullptr,
            m_forwardPhongPixelShader.GetAddressOf()
        ),
        "ModelRenderer failed to create forward Phong pixel shader."
    );

    const HRESULT forwardPhongInputLayoutResult = device->CreateInputLayout(
        kForwardPhongModelInputElements,
        static_cast<UINT>(std::size(kForwardPhongModelInputElements)),
        modelVertexShaderBlob->GetBufferPointer(),
        modelVertexShaderBlob->GetBufferSize(),
        m_forwardPhongModelInputLayout.GetAddressOf()
    );
    if (FAILED(forwardPhongInputLayoutResult))
    {
        const std::filesystem::path resolvedShaderPath =
            AssetPathResolver::Resolve(std::filesystem::path("Core/Shaders/ForwardPhong3D.hlsl"));
        char hexBuffer[24]{};
        std::snprintf(
            hexBuffer,
            sizeof(hexBuffer),
            "0x%08lX",
            static_cast<unsigned long>(static_cast<unsigned int>(forwardPhongInputLayoutResult))
        );
        throw std::runtime_error(
            std::string("ModelRenderer failed to create forward Phong model input layout. HRESULT: ")
            + hexBuffer + ". Compiled shader file: " + resolvedShaderPath.string()
        );
    }

    CreateForwardPhongDefaultWhiteDiffuseTexture(device, m_forwardPhongDefaultWhiteDiffuseTexture);

    CreateDynamicConstantBuffer(device, static_cast<UINT>(sizeof(CameraGpuConstants)), m_forwardPhongCameraConstantBuffer);
    CreateDynamicConstantBuffer(device, static_cast<UINT>(sizeof(ObjectGpuConstants)), m_forwardPhongObjectConstantBuffer);
    CreateDynamicConstantBuffer(device, static_cast<UINT>(sizeof(MaterialGpuConstants)), m_forwardPhongMaterialConstantBuffer);
    CreateDynamicConstantBuffer(device, static_cast<UINT>(sizeof(LightsGpuConstants)), m_forwardPhongLightsConstantBuffer);
}

void ModelRenderer::DrawModelShadowDepth(
    ID3D11RasterizerState *const shadowRasterizerState,
    const ModelAsset &model,
    const Matrix &world,
    const Matrix &lightViewProjection
) const
{
    if (m_graphics == nullptr || m_commonStates == nullptr)
    {
        throw std::logic_error("ModelRenderer::DrawModelShadowDepth called before Initialize.");
    }

    if (m_shadowDepthVertexShader == nullptr || m_shadowDepthInputLayout == nullptr || m_shadowDepthConstantBuffer == nullptr)
    {
        throw std::logic_error("ModelRenderer::DrawModelShadowDepth called before shadow depth resources were created.");
    }

    DirectX::Model *const drawableModel = model.Get();
    if (drawableModel == nullptr)
    {
        return;
    }

    ID3D11DeviceContext *const deviceContext = m_graphics->GetImmediateContext();
    if (deviceContext == nullptr)
    {
        throw std::logic_error("ModelRenderer::DrawModelShadowDepth failed: null device context.");
    }

    ShadowDepthObjectGpuConstants depthConstants{};
    depthConstants.World = world.Transpose();
    depthConstants.LightViewProjection = lightViewProjection.Transpose();

    D3D11_MAPPED_SUBRESOURCE mappedDepthConstants{};
    D3d11Helpers::ThrowIfFailed(
        deviceContext->Map(m_shadowDepthConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedDepthConstants),
        "ModelRenderer::DrawModelShadowDepth failed to map shadow depth constant buffer."
    );
    std::memcpy(mappedDepthConstants.pData, &depthConstants, sizeof(ShadowDepthObjectGpuConstants));
    deviceContext->Unmap(m_shadowDepthConstantBuffer.Get(), 0);

    ID3D11Buffer *const depthConstantBuffers[] = {m_shadowDepthConstantBuffer.Get()};
    deviceContext->VSSetShader(m_shadowDepthVertexShader.Get(), nullptr, 0);
    deviceContext->VSSetConstantBuffers(0, 1, depthConstantBuffers);
    deviceContext->PSSetShader(nullptr, nullptr, 0);
    deviceContext->IASetInputLayout(m_shadowDepthInputLayout.Get());

    if (shadowRasterizerState != nullptr)
    {
        deviceContext->RSSetState(shadowRasterizerState);
    }

    deviceContext->OMSetBlendState(m_commonStates->Opaque(), nullptr, 0xFFFFFFFFu);
    deviceContext->OMSetDepthStencilState(m_commonStates->DepthDefault(), 0u);

    constexpr UINT expectedStride = sizeof(DirectX::VertexPositionNormalTexture);

    for (const std::shared_ptr<DirectX::ModelMesh> &meshShared : drawableModel->meshes)
    {
        if (meshShared == nullptr)
        {
            continue;
        }

        for (const std::unique_ptr<DirectX::ModelMeshPart> &partUnique : meshShared->meshParts)
        {
            if (partUnique == nullptr)
            {
                continue;
            }

            const DirectX::ModelMeshPart &meshPart = *partUnique;
            if (meshPart.vertexStride != expectedStride)
            {
                continue;
            }

            const UINT stride = meshPart.vertexStride;
            const UINT bufferOffset = 0u;
            ID3D11Buffer *const vertexBuffers[] = {meshPart.vertexBuffer.Get()};
            const UINT strides[] = {stride};
            const UINT offsets[] = {bufferOffset};

            deviceContext->IASetVertexBuffers(0, 1, vertexBuffers, strides, offsets);
            deviceContext->IASetIndexBuffer(meshPart.indexBuffer.Get(), meshPart.indexFormat, 0);
            deviceContext->IASetPrimitiveTopology(meshPart.primitiveType);

            deviceContext->DrawIndexed(meshPart.indexCount, meshPart.startIndex, meshPart.vertexOffset);
        }
    }
}

void ModelRenderer::DrawModel(
    const ModelAsset &model,
    const Matrix &world,
    const Matrix &view,
    const Matrix &projection,
    const RenderMaterialParameters &material
) const
{
    if (m_graphics == nullptr || m_commonStates == nullptr)
    {
        throw std::logic_error("ModelRenderer::DrawModel called before Initialize.");
    }

    DirectX::Model *const drawableModel = model.Get();
    if (drawableModel == nullptr)
    {
        return;
    }

    ID3D11DeviceContext *const deviceContext = m_graphics->GetImmediateContext();
    if (deviceContext == nullptr)
    {
        throw std::logic_error("ModelRenderer::DrawModel failed: null device context.");
    }

    if (m_shadowBindingHost != nullptr
        && m_shadowBindingHost->GetActiveRenderPassKind() == RenderPassKind::DeferredGeometry)
    {
        DrawModelDeferredGeometry(
            model,
            world,
            view,
            projection,
            CameraWorldPositionFromViewMatrix(view),
            material
        );
        return;
    }

    if (m_shadowBindingHost == nullptr || m_shadowBindingHost->ShouldBindMainRenderTargetsForDraw())
    {
        m_graphics->BindMainRenderTargets();
    }

    const DirectX::XMFLOAT4 &baseColorFloat4 = static_cast<const DirectX::XMFLOAT4 &>(material.BaseColor);
    const DirectX::XMVECTOR colorAndAlpha = DirectX::XMLoadFloat4(&baseColorFloat4);

    drawableModel->UpdateEffects(
        [&](DirectX::IEffect *effectTarget)
        {
            ApplyTintToEffect(effectTarget, colorAndAlpha, material);
        }
    );

    drawableModel->Draw(
        deviceContext,
        *m_commonStates,
        world,
        view,
        projection,
        material.Wireframe
    );
}

void ModelRenderer::DrawModelDeferredGeometry(
    const ModelAsset &model,
    const Matrix &world,
    const Matrix &view,
    const Matrix &projection,
    const Vector3 &cameraWorldPosition,
    const RenderMaterialParameters &material
) const
{
    if (m_graphics == nullptr || m_commonStates == nullptr)
    {
        throw std::logic_error("ModelRenderer::DrawModelDeferredGeometry called before Initialize.");
    }

    if (m_deferredGeometryModelVertexShader == nullptr
        || m_deferredGeometryPixelShader == nullptr
        || m_deferredGeometryModelInputLayout == nullptr)
    {
        throw std::logic_error("ModelRenderer::DrawModelDeferredGeometry called before deferred resources were created.");
    }

    DirectX::Model *const drawableModel = model.Get();
    if (drawableModel == nullptr)
    {
        return;
    }

    ID3D11DeviceContext *const deviceContext = m_graphics->GetImmediateContext();
    if (deviceContext == nullptr)
    {
        throw std::logic_error("ModelRenderer::DrawModelDeferredGeometry failed: null device context.");
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
    FillMaterialGpuConstantsFromRenderMaterial(material, materialConstants);

    UploadConstantBuffer(
        deviceContext,
        m_forwardPhongCameraConstantBuffer.Get(),
        &cameraConstants,
        sizeof(CameraGpuConstants),
        "ModelRenderer::DrawModelDeferredGeometry failed to map camera constant buffer."
    );
    UploadConstantBuffer(
        deviceContext,
        m_forwardPhongObjectConstantBuffer.Get(),
        &objectConstants,
        sizeof(ObjectGpuConstants),
        "ModelRenderer::DrawModelDeferredGeometry failed to map object constant buffer."
    );
    UploadConstantBuffer(
        deviceContext,
        m_forwardPhongMaterialConstantBuffer.Get(),
        &materialConstants,
        sizeof(MaterialGpuConstants),
        "ModelRenderer::DrawModelDeferredGeometry failed to map material constant buffer."
    );

    ID3D11Buffer *const constantBuffers[] =
    {
        m_forwardPhongCameraConstantBuffer.Get(),
        m_forwardPhongObjectConstantBuffer.Get(),
        m_forwardPhongMaterialConstantBuffer.Get()
    };

    deviceContext->VSSetShader(m_deferredGeometryModelVertexShader.Get(), nullptr, 0);
    deviceContext->PSSetShader(m_deferredGeometryPixelShader.Get(), nullptr, 0);
    deviceContext->VSSetConstantBuffers(0, 3, constantBuffers);
    deviceContext->PSSetConstantBuffers(0, 3, constantBuffers);

    ID3D11SamplerState *const diffuseSamplerStates[] = {m_commonStates->LinearWrap()};
    deviceContext->PSSetSamplers(0, 1, diffuseSamplerStates);

    deviceContext->IASetInputLayout(m_deferredGeometryModelInputLayout.Get());

    deviceContext->OMSetBlendState(m_commonStates->Opaque(), nullptr, 0xFFFFFFFFu);
    deviceContext->OMSetDepthStencilState(m_commonStates->DepthDefault(), 0u);

    constexpr UINT expectedStride = sizeof(DirectX::VertexPositionNormalTexture);

    std::size_t flatMeshPartIndex = 0u;

    for (const std::shared_ptr<DirectX::ModelMesh> &meshShared : drawableModel->meshes)
    {
        if (meshShared == nullptr)
        {
            continue;
        }

        deviceContext->RSSetState(ResolveRasterizerForMesh(*m_commonStates, *meshShared, material));

        for (const std::unique_ptr<DirectX::ModelMeshPart> &partUnique : meshShared->meshParts)
        {
            if (partUnique == nullptr)
            {
                continue;
            }

            const DirectX::ModelMeshPart &meshPart = *partUnique;

            ID3D11ShaderResourceView *diffuseShaderResourceView = model.TryGetDiffuseTextureForMeshPart(flatMeshPartIndex);
            ++flatMeshPartIndex;
            if (diffuseShaderResourceView == nullptr)
            {
                diffuseShaderResourceView = m_forwardPhongDefaultWhiteDiffuseTexture.Get();
            }

            if (meshPart.vertexStride != expectedStride)
            {
                continue;
            }

            deviceContext->PSSetShaderResources(0, 1, &diffuseShaderResourceView);

            const UINT stride = meshPart.vertexStride;
            const UINT bufferOffset = 0u;
            ID3D11Buffer *const vertexBuffers[] = {meshPart.vertexBuffer.Get()};
            const UINT strides[] = {stride};
            const UINT offsets[] = {bufferOffset};

            deviceContext->IASetVertexBuffers(0, 1, vertexBuffers, strides, offsets);
            deviceContext->IASetIndexBuffer(meshPart.indexBuffer.Get(), meshPart.indexFormat, 0);
            deviceContext->IASetPrimitiveTopology(meshPart.primitiveType);

            deviceContext->DrawIndexed(meshPart.indexCount, meshPart.startIndex, meshPart.vertexOffset);
        }
    }

    ID3D11ShaderResourceView *const nullShaderResourceViews[] = {nullptr};
    deviceContext->PSSetShaderResources(0, 1, nullShaderResourceViews);
    deviceContext->RSSetState(nullptr);
}

void ModelRenderer::DrawModelLit(
    const ModelAsset &model,
    const Matrix &world,
    const Matrix &view,
    const Matrix &projection,
    const Vector3 &cameraWorldPosition,
    const SceneLightingDescriptor3D &lighting,
    const RenderMaterialParameters &material
) const
{
    if (m_graphics == nullptr || m_commonStates == nullptr)
    {
        throw std::logic_error("ModelRenderer::DrawModelLit called before Initialize.");
    }

    if (m_shadowBindingHost == nullptr)
    {
        throw std::logic_error("ModelRenderer::DrawModelLit requires a shadow binding host from RenderContext::Initialize.");
    }

    if (m_forwardPhongModelVertexShader == nullptr || m_forwardPhongPixelShader == nullptr || m_forwardPhongModelInputLayout == nullptr)
    {
        throw std::logic_error("ModelRenderer::DrawModelLit called before forward Phong resources were created.");
    }

    DirectX::Model *const drawableModel = model.Get();
    if (drawableModel == nullptr)
    {
        return;
    }

    ID3D11DeviceContext *const deviceContext = m_graphics->GetImmediateContext();
    if (deviceContext == nullptr)
    {
        throw std::logic_error("ModelRenderer::DrawModelLit failed: null device context.");
    }

    if (m_shadowBindingHost->GetActiveRenderPassKind() == RenderPassKind::DeferredGeometry)
    {
        DrawModelDeferredGeometry(
            model,
            world,
            view,
            projection,
            cameraWorldPosition,
            material
        );
        return;
    }

    if (m_shadowBindingHost == nullptr || m_shadowBindingHost->ShouldBindMainRenderTargetsForDraw())
    {
        m_graphics->BindMainRenderTargets();
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
    FillMaterialGpuConstantsFromRenderMaterial(material, materialConstants);

    LightsGpuConstants lightsConstants{};
    FillLightsGpuConstants(lighting, lightsConstants);

    ForwardPhongUploadConstantBuffers(
        deviceContext,
        cameraConstants,
        objectConstants,
        materialConstants,
        lightsConstants,
        m_forwardPhongCameraConstantBuffer.Get(),
        m_forwardPhongObjectConstantBuffer.Get(),
        m_forwardPhongMaterialConstantBuffer.Get(),
        m_forwardPhongLightsConstantBuffer.Get()
    );

    ID3D11Buffer *const constantBuffers[] =
    {
        m_forwardPhongCameraConstantBuffer.Get(),
        m_forwardPhongObjectConstantBuffer.Get(),
        m_forwardPhongMaterialConstantBuffer.Get(),
        m_forwardPhongLightsConstantBuffer.Get()
    };

    deviceContext->VSSetShader(m_forwardPhongModelVertexShader.Get(), nullptr, 0);
    deviceContext->PSSetShader(m_forwardPhongPixelShader.Get(), nullptr, 0);
    deviceContext->VSSetConstantBuffers(0, 4, constantBuffers);
    deviceContext->PSSetConstantBuffers(0, 4, constantBuffers);
    m_shadowBindingHost->BindForwardPhongShadowRegisters(deviceContext);

    ID3D11SamplerState *const diffuseSamplerStates[] = {m_commonStates->LinearWrap()};
    deviceContext->PSSetSamplers(1, 1, diffuseSamplerStates);

    deviceContext->IASetInputLayout(m_forwardPhongModelInputLayout.Get());

    deviceContext->OMSetBlendState(m_commonStates->Opaque(), nullptr, 0xFFFFFFFFu);
    deviceContext->OMSetDepthStencilState(m_commonStates->DepthDefault(), 0u);

    constexpr UINT expectedStride = sizeof(DirectX::VertexPositionNormalTexture);

    std::size_t flatMeshPartIndex = 0u;

    for (const std::shared_ptr<DirectX::ModelMesh> &meshShared : drawableModel->meshes)
    {
        if (meshShared == nullptr)
        {
            continue;
        }

        deviceContext->RSSetState(ResolveRasterizerForMesh(*m_commonStates, *meshShared, material));

        for (const std::unique_ptr<DirectX::ModelMeshPart> &partUnique : meshShared->meshParts)
        {
            if (partUnique == nullptr)
            {
                continue;
            }

            const DirectX::ModelMeshPart &meshPart = *partUnique;

            ID3D11ShaderResourceView *diffuseShaderResourceView = model.TryGetDiffuseTextureForMeshPart(flatMeshPartIndex);
            ++flatMeshPartIndex;
            if (diffuseShaderResourceView == nullptr)
            {
                diffuseShaderResourceView = m_forwardPhongDefaultWhiteDiffuseTexture.Get();
            }

            if (meshPart.vertexStride != expectedStride)
            {
                continue;
            }

            deviceContext->PSSetShaderResources(1, 1, &diffuseShaderResourceView);

            const UINT stride = meshPart.vertexStride;
            const UINT bufferOffset = 0u;
            ID3D11Buffer *const vertexBuffers[] = {meshPart.vertexBuffer.Get()};
            const UINT strides[] = {stride};
            const UINT offsets[] = {bufferOffset};

            deviceContext->IASetVertexBuffers(0, 1, vertexBuffers, strides, offsets);
            deviceContext->IASetIndexBuffer(meshPart.indexBuffer.Get(), meshPart.indexFormat, 0);
            deviceContext->IASetPrimitiveTopology(meshPart.primitiveType);

            deviceContext->DrawIndexed(meshPart.indexCount, meshPart.startIndex, meshPart.vertexOffset);
        }
    }

    m_shadowBindingHost->UnbindForwardPhongShadowShaderResource(deviceContext);

    deviceContext->RSSetState(nullptr);
}
