#include "Core/Graphics/Picking/GBufferPickingService.h"

#include "Core/Graphics/Camera.h"
#include "Core/Graphics/D3d11Helpers.h"
#include "Core/Graphics/GraphicsDevice.h"
#include "Core/Graphics/Rendering/Deferred/GBufferLayout.h"
#include "Core/Graphics/Rendering/Deferred/GBufferResources.h"

#include <cmath>
#include <cstring>
#include <stdexcept>

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;

void GBufferPickingService::Initialize(GraphicsDevice &graphics)
{
    ID3D11Device *const device = graphics.GetDevice();
    if (device == nullptr)
    {
        return;
    }

    EnsureStagingTextures(device);
    m_initialized = true;
}

void GBufferPickingService::Shutdown() noexcept
{
    m_stagingObjectId.Reset();
    m_stagingNormal.Reset();
    m_stagingDepth.Reset();
    m_initialized = false;
}

void GBufferPickingService::EnsureStagingTextures(ID3D11Device *const device)
{
    if (device == nullptr)
    {
        return;
    }

    auto createStaging1By1 = [&](const DXGI_FORMAT format, Microsoft::WRL::ComPtr<ID3D11Texture2D> &outTexture) {
        if (outTexture != nullptr)
        {
            D3D11_TEXTURE2D_DESC existing{};
            outTexture->GetDesc(&existing);
            if (existing.Format == format && existing.Width == 1u && existing.Height == 1u)
            {
                return;
            }
        }

        outTexture.Reset();
        D3D11_TEXTURE2D_DESC description{};
        description.Width = 1u;
        description.Height = 1u;
        description.MipLevels = 1u;
        description.ArraySize = 1u;
        description.Format = format;
        description.SampleDesc.Count = 1u;
        description.SampleDesc.Quality = 0u;
        description.Usage = D3D11_USAGE_STAGING;
        description.BindFlags = 0u;
        description.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        description.MiscFlags = 0u;

        D3d11Helpers::ThrowIfFailed(
            device->CreateTexture2D(&description, nullptr, outTexture.GetAddressOf()),
            "GBufferPickingService failed to create staging texture."
        );
    };

    createStaging1By1(DXGI_FORMAT_R32_UINT, m_stagingObjectId);
    createStaging1By1(kGBufferNormalFormat, m_stagingNormal);
    createStaging1By1(DXGI_FORMAT_R32_FLOAT, m_stagingDepth);
}

GBufferPickResult GBufferPickingService::Pick(
    GraphicsDevice &graphics,
    const GBufferResources &gBuffer,
    const Camera &camera,
    const float viewportAspectRatio,
    const int screenX,
    const int screenY
)
{
    GBufferPickResult result{};
    result.ScreenX = screenX;
    result.ScreenY = screenY;

    if (!m_initialized || !gBuffer.IsCreated())
    {
        return result;
    }

    ID3D11Device *const device = graphics.GetDevice();
    ID3D11DeviceContext *const context = graphics.GetImmediateContext();
    if (device == nullptr || context == nullptr)
    {
        return result;
    }

    const std::uint32_t textureWidth = gBuffer.GetWidth();
    const std::uint32_t textureHeight = gBuffer.GetHeight();
    if (textureWidth == 0u || textureHeight == 0u)
    {
        return result;
    }

    if (!(viewportAspectRatio > 0.0f) || std::isnan(viewportAspectRatio) || std::isinf(viewportAspectRatio))
    {
        return result;
    }

    if (screenX < 0 || screenY < 0
        || screenX >= static_cast<int>(textureWidth)
        || screenY >= static_cast<int>(textureHeight))
    {
        return result;
    }

    EnsureStagingTextures(device);

    if (m_stagingObjectId == nullptr || m_stagingNormal == nullptr || m_stagingDepth == nullptr)
    {
        return result;
    }

    D3D11_BOX sourceBox{};
    sourceBox.left = static_cast<std::uint32_t>(screenX);
    sourceBox.top = static_cast<std::uint32_t>(screenY);
    sourceBox.front = 0u;
    sourceBox.right = sourceBox.left + 1u;
    sourceBox.bottom = sourceBox.top + 1u;
    sourceBox.back = 1u;

    ID3D11Texture2D *const objectIdTexture = gBuffer.GetObjectIdTarget().GetTexture();
    ID3D11Texture2D *const normalTexture = gBuffer.GetNormalTarget().GetTexture();
    ID3D11Texture2D *const depthTexture = gBuffer.GetSceneDepthTarget().GetTexture();

    context->CopySubresourceRegion(m_stagingObjectId.Get(), 0u, 0, 0, 0, objectIdTexture, 0u, &sourceBox);
    context->CopySubresourceRegion(m_stagingNormal.Get(), 0u, 0, 0, 0, normalTexture, 0u, &sourceBox);
    context->CopySubresourceRegion(m_stagingDepth.Get(), 0u, 0, 0, 0, depthTexture, 0u, &sourceBox);

    std::uint32_t ObjectIdValue = 0u;
    {
        D3D11_MAPPED_SUBRESOURCE mapped{};
        D3d11Helpers::ThrowIfFailed(
            context->Map(m_stagingObjectId.Get(), 0u, D3D11_MAP_READ, 0u, &mapped),
            "GBufferPickingService failed to map object id staging texture."
        );
        ObjectIdValue = *reinterpret_cast<const std::uint32_t *>(mapped.pData);
        context->Unmap(m_stagingObjectId.Get(), 0u);
    }

    std::uint32_t PackedNormal = 0u;
    {
        D3D11_MAPPED_SUBRESOURCE mapped{};
        D3d11Helpers::ThrowIfFailed(
            context->Map(m_stagingNormal.Get(), 0u, D3D11_MAP_READ, 0u, &mapped),
            "GBufferPickingService failed to map normal staging texture."
        );
        PackedNormal = *reinterpret_cast<const std::uint32_t *>(mapped.pData);
        context->Unmap(m_stagingNormal.Get(), 0u);
    }

    float DeviceDepth = 0.0f;
    {
        D3D11_MAPPED_SUBRESOURCE mapped{};
        D3d11Helpers::ThrowIfFailed(
            context->Map(m_stagingDepth.Get(), 0u, D3D11_MAP_READ, 0u, &mapped),
            "GBufferPickingService failed to map depth staging texture."
        );
        DeviceDepth = *reinterpret_cast<const float *>(mapped.pData);
        context->Unmap(m_stagingDepth.Get(), 0u);
    }

    result.ObjectId = ObjectIdValue;
    result.Depth = DeviceDepth;

    if (ObjectIdValue == 0u || DeviceDepth >= 1.0f)
    {
        return result;
    }

    constexpr float kInverseTenBit = 1.0f / 1023.0f;
    const float encodedNormalX = static_cast<float>(PackedNormal & 1023u) * kInverseTenBit;
    const float encodedNormalY = static_cast<float>((PackedNormal >> 10) & 1023u) * kInverseTenBit;
    const float encodedNormalZ = static_cast<float>((PackedNormal >> 20) & 1023u) * kInverseTenBit;
    Vector3 worldNormal(
        encodedNormalX * 2.0f - 1.0f,
        encodedNormalY * 2.0f - 1.0f,
        encodedNormalZ * 2.0f - 1.0f
    );
    worldNormal.Normalize();
    result.WorldNormal = worldNormal;

    const Matrix viewMatrix = camera.GetViewMatrix();
    const Matrix projectionMatrix = camera.GetProjectionMatrix(viewportAspectRatio);
    const Matrix inverseProjection = projectionMatrix.Invert();
    const Matrix inverseView = viewMatrix.Invert();

    const float normalizedDeviceCoordinateX =
        (static_cast<float>(screenX) + 0.5f) / static_cast<float>(textureWidth) * 2.0f - 1.0f;
    const float normalizedDeviceCoordinateY =
        1.0f - (static_cast<float>(screenY) + 0.5f) / static_cast<float>(textureHeight) * 2.0f;

    Vector4 clipPosition(normalizedDeviceCoordinateX, normalizedDeviceCoordinateY, DeviceDepth, 1.0f);
    Vector4 viewPosition = Vector4::Transform(clipPosition, inverseProjection);
    const float viewW = std::fabs(viewPosition.w);
    if (viewW > 1.0e-5f)
    {
        viewPosition.x /= viewPosition.w;
        viewPosition.y /= viewPosition.w;
        viewPosition.z /= viewPosition.w;
    }

    Vector4 worldPosition4 = Vector4::Transform(
        Vector4(viewPosition.x, viewPosition.y, viewPosition.z, 1.0f),
        inverseView
    );
    const float worldW = std::fabs(worldPosition4.w);
    if (worldW > 1.0e-5f)
    {
        worldPosition4.x /= worldPosition4.w;
        worldPosition4.y /= worldPosition4.w;
        worldPosition4.z /= worldPosition4.w;
    }

    result.WorldPosition = Vector3(worldPosition4.x, worldPosition4.y, worldPosition4.z);
    result.Hit = true;
    return result;
}
