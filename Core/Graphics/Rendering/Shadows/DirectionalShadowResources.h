#ifndef PINGPONG_DIRECTIONALSHADOWRESOURCES_H
#define PINGPONG_DIRECTIONALSHADOWRESOURCES_H

#include "Core/Graphics/Rendering/Shadows/DepthRenderTarget2D.h"
#include "Core/Graphics/Rendering/Shadows/ShadowComparisonSamplerState.h"
#include "Core/Graphics/Rendering/Shadows/ShadowDepthPassRasterizerState.h"
#include "Core/Graphics/Rendering/Shadows/ShadowShaderConstants3D.h"

#include <d3d11.h>
#include <wrl/client.h>

class GraphicsDevice;

class DirectionalShadowResources final
{
public:
    void Initialize(GraphicsDevice &graphicsDevice);

    void Shutdown() noexcept;

    void Resize(GraphicsDevice &graphicsDevice);

    void UploadShadowCascadeConstants(
        ID3D11DeviceContext *deviceContext,
        const ShadowCascadeConstantsGpu &constants
    ) const;

    void UploadShadowSamplingConstants(
        ID3D11DeviceContext *deviceContext,
        const ShadowSamplingGpuConstants &constants
    ) const;

    [[nodiscard]] DepthRenderTarget2D &GetShadowMap() noexcept;

    [[nodiscard]] const DepthRenderTarget2D &GetShadowMap() const noexcept;

    [[nodiscard]] ID3D11Buffer *GetShadowCascadeConstantBuffer() const noexcept;

    [[nodiscard]] ID3D11Buffer *GetShadowSamplingConstantBuffer() const noexcept;

    [[nodiscard]] ID3D11SamplerState *GetShadowComparisonSampler() const noexcept;

    [[nodiscard]] ID3D11RasterizerState *GetShadowPassRasterizerState() const noexcept;

    [[nodiscard]] float GetOrthographicWidth() const noexcept;

    [[nodiscard]] float GetOrthographicHeight() const noexcept;

    [[nodiscard]] float GetLightEyeDistanceFromFocus() const noexcept;

    [[nodiscard]] float GetNearPlaneDistance() const noexcept;

    [[nodiscard]] float GetFarPlaneDistance() const noexcept;

    [[nodiscard]] std::uint32_t GetCascadeCount() const noexcept;

    [[nodiscard]] std::uint32_t GetShadowAtlasSizePixels() const noexcept;

    [[nodiscard]] std::uint32_t GetCascadeTileSizePixels() const noexcept;

    [[nodiscard]] float GetCascadeSplitLambda() const noexcept;

    [[nodiscard]] float GetEffectiveShadowFarClamp() const noexcept;

private:
    void CreateConstantBuffers(ID3D11Device *device);

private:
    DepthRenderTarget2D m_shadowMap{};
    ShadowComparisonSamplerState m_comparisonSampler{};
    ShadowDepthPassRasterizerState m_shadowPassRasterizer{};
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_shadowCascadeConstantBuffer{};
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_shadowSamplingConstantBuffer{};
    std::uint32_t m_shadowAtlasSizePixels{2048u};
    std::uint32_t m_cascadeCount{4u};
    float m_cascadeSplitLambda{0.65f};
    float m_effectiveShadowFarClamp{160.0f};
    float m_orthographicWidth{140.0f};
    float m_orthographicHeight{140.0f};
    float m_lightEyeDistanceFromFocus{90.0f};
    float m_nearPlaneDistance{0.5f};
    float m_farPlaneDistance{320.0f};
};

#endif
