#include "RenderContext.h"

#include "Core/Gameplay/Entity.h"
#include "Core/Gameplay/GameplayComponents.h"
#include "Core/Gameplay/Scene.h"
#include "Core/Graphics/Camera.h"
#include "Core/Graphics/GraphicsDevice.h"
#include "Core/Graphics/ModelAsset.h"
#include "Core/Graphics/Rendering/FrameRenderTypes.h"
#include "Core/Graphics/Rendering/Lighting/LightTypes3D.h"
#include "Core/Graphics/Rendering/Lighting/SceneLighting3D.h"
#include "Core/Graphics/Rendering/Pipeline/FramePassRenderContext.h"
#include "Core/Graphics/Rendering/Pipeline/Passes/DeferredCompositeRenderPass.h"
#include "Core/Graphics/Rendering/Pipeline/Passes/DeferredGeometryRenderPass.h"
#include "Core/Graphics/Rendering/Pipeline/Passes/GBufferDebugRenderPass.h"
#include "Core/Graphics/Rendering/Pipeline/Passes/DeferredLightingRenderPass.h"
#include "Core/Graphics/Rendering/Pipeline/Passes/DebugOverlayRenderPass.h"
#include "Core/Graphics/Rendering/Pipeline/Passes/GameRenderPass.h"
#include "Core/Graphics/Rendering/Pipeline/Passes/MainGeometryBindRenderPass.h"
#include "Core/Graphics/Rendering/Pipeline/Passes/ParticlesRenderPass.h"
#include "Core/Graphics/Rendering/Pipeline/Passes/ShadowDepthRenderPass.h"
#include "Core/Graphics/Rendering/Pipeline/Passes/UserInterfaceRenderPass.h"
#include "Core/Graphics/Rendering/Shadows/CascadedShadowMapMath.h"

#include <DirectXMath.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <memory>
#include <utility>

RenderContext::~RenderContext()
{
    m_gpuParticleSystem.Shutdown();
    m_directionalShadowResources.Shutdown();
}

void RenderContext::Initialize(GraphicsDevice &graphics)
{
    m_graphics = &graphics;

    m_frameRenderResources.Initialize(graphics);
    m_frameRenderResources.SetDeferredFrameResources(&m_deferredFrameResources);

    m_frameRenderer.Initialize(graphics);
    m_deferredFrameResources.CreateOrResize(graphics);
    m_directionalShadowResources.Initialize(graphics);
    m_gpuParticleSystem.Initialize(graphics, 4096u);
    m_shapeRenderer2D.Initialize(graphics);
    m_primitiveRenderer3D.Initialize(graphics);
    m_primitiveRenderer3D.SetShadowBindingHost(this);
    m_modelRenderer.Initialize(graphics);
    m_modelRenderer.SetShadowBindingHost(this);
    m_sceneRenderer3D.Initialize(m_primitiveRenderer3D, m_modelRenderer);

    m_frameRenderPipeline.Initialize(graphics, m_frameRenderResources);
}

void RenderContext::BeginRenderFrame()
{
    m_directionalShadowPassCompletedThisFrame = false;
    InvalidateDirectionalShadowPass();
}

void RenderContext::PrepareDirectionalShadowPass(Scene &scene, Camera &camera)
{
    if (m_graphics == nullptr)
    {
        return;
    }

    if (m_directionalShadowPassCompletedThisFrame)
    {
        return;
    }

    ID3D11DeviceContext *const deviceContext = m_graphics->GetImmediateContext();
    if (deviceContext == nullptr)
    {
        return;
    }

    if (!scene.GetDirectionalShadowMappingEnabled())
    {
        InvalidateDirectionalShadowPass();
        return;
    }

    const SceneLightingDescriptor3D &lighting = scene.GetSceneLightingDescriptor();
    const DirectionalLight3D *shadowDirectional = nullptr;
    for (const DirectionalLight3D &directional : lighting.DirectionalLights)
    {
        if (directional.Enabled)
        {
            shadowDirectional = &directional;
            break;
        }
    }

    if (shadowDirectional == nullptr)
    {
        InvalidateDirectionalShadowPass();
        return;
    }

    std::uint32_t shadowPackedDirectionalIndex = 0u;
    for (const DirectionalLight3D &directional : lighting.DirectionalLights)
    {
        if (!directional.Enabled)
        {
            continue;
        }
        if (&directional == shadowDirectional)
        {
            break;
        }
        shadowPackedDirectionalIndex++;
    }

    DirectX::SimpleMath::Vector3 lightAxis = shadowDirectional->Direction;
    lightAxis.Normalize();

    const int framebufferWidth = m_graphics->GetWidth();
    const int framebufferHeight = m_graphics->GetHeight();
    if (framebufferWidth <= 0 || framebufferHeight <= 0)
    {
        InvalidateDirectionalShadowPass();
        return;
    }

    const float aspectRatio =
        static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight);

    const DirectX::SimpleMath::Matrix viewMatrix = camera.GetViewMatrix();

    const float effectiveFar = (std::min)(
        camera.GetFarPlane(),
        m_directionalShadowResources.GetEffectiveShadowFarClamp()
    );
    const float effectiveNear = camera.GetNearPlane();

    const std::uint32_t cascadeCount = m_directionalShadowResources.GetCascadeCount();
    const CascadedShadowSplitCpu splitCpu = ComputePracticalCascadeSplits(
        effectiveNear,
        effectiveFar,
        cascadeCount,
        m_directionalShadowResources.GetCascadeSplitLambda()
    );

    const float verticalFieldOfViewRadians = DirectX::XMConvertToRadians(camera.GetVerticalFieldOfViewDegrees());
    const std::uint32_t cascadeTilePixels = m_directionalShadowResources.GetCascadeTileSizePixels();

    std::array<DirectX::SimpleMath::Matrix, kMaximumShadowCascades> cascadeLightViewProjections{};
    for (std::uint32_t cascadeIndex = 0u; cascadeIndex < cascadeCount; ++cascadeIndex)
    {
        const float sliceNearZ = splitCpu.ViewSpaceSplitBoundaries[cascadeIndex];
        const float sliceFarZ = splitCpu.ViewSpaceSplitBoundaries[cascadeIndex + 1u];
        cascadeLightViewProjections[cascadeIndex] = BuildDirectionalCascadeLightViewProjection(
            lightAxis,
            viewMatrix,
            sliceNearZ,
            sliceFarZ,
            verticalFieldOfViewRadians,
            aspectRatio,
            cascadeTilePixels,
            2.0f
        );
    }

    ShadowCascadeConstantsGpu cascadeCpu{};
    for (std::uint32_t cascadeIndex = 0u; cascadeIndex < cascadeCount; ++cascadeIndex)
    {
        cascadeCpu.LightViewProjection[cascadeIndex] = cascadeLightViewProjections[cascadeIndex].Transpose();
    }
    cascadeCpu.CascadeSplits = DirectX::XMFLOAT4(
        splitCpu.ViewSpaceSplitBoundaries[1],
        splitCpu.ViewSpaceSplitBoundaries[2],
        splitCpu.ViewSpaceSplitBoundaries[3],
        splitCpu.ViewSpaceSplitBoundaries[4]
    );
    cascadeCpu.CascadeCount = cascadeCount;

    m_directionalShadowPassCompletedThisFrame = true;

    const float inverseAtlasWidth =
        1.0f / static_cast<float>(m_directionalShadowResources.GetShadowAtlasSizePixels());

    constexpr float kConstantDepthBias = 0.0008f;
    constexpr float kSlopeScaledDepthBias = 0.0015f;
    constexpr float kNormalOffsetWorldUnits = 0.03f;
    constexpr float kPcfRadiusTexels = 2.0f;

    ShadowSamplingGpuConstants samplingCpu{};
    samplingCpu.DepthBiasAndPcfKernel = DirectX::XMFLOAT4(
        kConstantDepthBias,
        kSlopeScaledDepthBias,
        kNormalOffsetWorldUnits,
        kPcfRadiusTexels
    );
    samplingCpu.InvShadowMapTexelSize = DirectX::XMFLOAT2(inverseAtlasWidth, inverseAtlasWidth);
    samplingCpu.ShadowEnabled = 1u;
    samplingCpu.ShadowedDirectionalLightGpuIndex = shadowPackedDirectionalIndex;
    samplingCpu.CascadeCount = cascadeCount;
    samplingCpu.ShadowCascadeDebugMode = scene.GetShadowCascadeDebugVisualizationEnabled() ? 1u : 0u;

    m_directionalShadowResources.UploadShadowCascadeConstants(deviceContext, cascadeCpu);
    m_directionalShadowResources.UploadShadowSamplingConstants(deviceContext, samplingCpu);

    ID3D11ShaderResourceView *const nullShaderResourceViews[8] = {};
    deviceContext->PSSetShaderResources(0, 8, nullShaderResourceViews);

    m_frameRenderer.EnterPass(RenderPassKind::ShadowCapture);

    DepthRenderTarget2D &shadowTarget = m_directionalShadowResources.GetShadowMap();
    ID3D11DepthStencilView *const shadowDepthStencilView = shadowTarget.GetDepthStencilView();

    m_graphics->BindDepthOnlyRenderTarget(shadowDepthStencilView);
    m_graphics->SetViewportPixels(
        static_cast<float>(shadowTarget.GetWidth()),
        static_cast<float>(shadowTarget.GetHeight())
    );
    m_graphics->ClearDepthStencilView(shadowDepthStencilView, 1.0f, 0u);

    ID3D11RasterizerState *const shadowRasterizerState = m_directionalShadowResources.GetShadowPassRasterizerState();

    for (std::uint32_t cascadeIndex = 0u; cascadeIndex < cascadeCount; ++cascadeIndex)
    {
        const float tilePixelsFloat = static_cast<float>(cascadeTilePixels);
        const float topLeftX = static_cast<float>(cascadeIndex % 2u) * tilePixelsFloat;
        const float topLeftY = static_cast<float>(cascadeIndex / 2u) * tilePixelsFloat;
        m_graphics->SetViewportPixels(tilePixelsFloat, tilePixelsFloat, topLeftX, topLeftY);

        scene.ForEachEntityWithTransformAndModel([&](Entity &entity) {
            ModelComponent *const model = entity.TryGetModelComponent();
            TransformComponent *const transform = entity.TryGetTransformComponent();
            if (model == nullptr || transform == nullptr)
            {
                return;
            }
            if (!model->Visible || !model->CastsShadow || model->Asset == nullptr)
            {
                return;
            }

            m_modelRenderer.DrawModelShadowDepth(
                shadowRasterizerState,
                *model->Asset,
                transform->WorldMatrix,
                cascadeLightViewProjections[cascadeIndex]
            );
        });
    }

    deviceContext->RSSetState(nullptr);
    m_graphics->BindMainRenderTargets();
    m_graphics->SetMainViewport();
    m_frameRenderer.LeavePass();
}

void RenderContext::InvalidateDirectionalShadowPass()
{
    if (m_graphics == nullptr)
    {
        return;
    }

    ID3D11DeviceContext *const deviceContext = m_graphics->GetImmediateContext();
    if (deviceContext == nullptr)
    {
        return;
    }

    ShadowSamplingGpuConstants samplingCpu{};
    samplingCpu.ShadowEnabled = 0u;
    samplingCpu.CascadeCount = 0u;
    samplingCpu.ShadowCascadeDebugMode = 0u;
    m_directionalShadowResources.UploadShadowSamplingConstants(deviceContext, samplingCpu);

    ShadowCascadeConstantsGpu cascadeCpu{};
    cascadeCpu.CascadeCount = 0u;
    m_directionalShadowResources.UploadShadowCascadeConstants(deviceContext, cascadeCpu);
}

void RenderContext::BindForwardPhongShadowRegisters(ID3D11DeviceContext *const deviceContext) const
{
    BindDirectionalShadowRegisters(deviceContext, 0u, 0u);
}

void RenderContext::BindDirectionalShadowRegisters(
    ID3D11DeviceContext *const deviceContext,
    const UINT shadowMapShaderResourceSlot,
    const UINT comparisonSamplerSlot
) const
{
    if (deviceContext == nullptr)
    {
        return;
    }

    ID3D11Buffer *const shadowConstantBuffers[] =
    {
        m_directionalShadowResources.GetShadowCascadeConstantBuffer(),
        m_directionalShadowResources.GetShadowSamplingConstantBuffer()
    };
    deviceContext->PSSetConstantBuffers(4, 2, shadowConstantBuffers);

    ID3D11ShaderResourceView *const shadowMapShaderResourceViews[] =
    {
        m_directionalShadowResources.GetShadowMap().GetShaderResourceView()
    };
    deviceContext->PSSetShaderResources(shadowMapShaderResourceSlot, 1, shadowMapShaderResourceViews);

    ID3D11SamplerState *const shadowSamplers[] =
    {
        m_directionalShadowResources.GetShadowComparisonSampler()
    };
    deviceContext->PSSetSamplers(comparisonSamplerSlot, 1, shadowSamplers);
}

void RenderContext::UnbindForwardPhongShadowShaderResource(ID3D11DeviceContext *const deviceContext) const
{
    UnbindDirectionalShadowShaderResource(deviceContext, 0u);
}

void RenderContext::UnbindDirectionalShadowShaderResource(
    ID3D11DeviceContext *const deviceContext,
    const UINT shadowMapShaderResourceSlot
) const
{
    if (deviceContext == nullptr)
    {
        return;
    }

    ID3D11ShaderResourceView *const nullShaderResourceViews[] = {nullptr};
    deviceContext->PSSetShaderResources(shadowMapShaderResourceSlot, 1, nullShaderResourceViews);
}

void RenderContext::ResizeDeferredResources()
{
    if (m_graphics == nullptr)
    {
        return;
    }

    m_deferredFrameResources.CreateOrResize(*m_graphics);
    m_directionalShadowResources.Resize(*m_graphics);
    m_frameRenderResources.OnBackbufferResize(m_graphics->GetWidth(), m_graphics->GetHeight());
    m_frameRenderPipeline.Resize(m_graphics->GetWidth(), m_graphics->GetHeight());
}

ShapeRenderer2D &RenderContext::GetShapeRenderer2D() noexcept
{
    return m_shapeRenderer2D;
}

const ShapeRenderer2D &RenderContext::GetShapeRenderer2D() const noexcept
{
    return m_shapeRenderer2D;
}

PrimitiveRenderer3D &RenderContext::GetPrimitiveRenderer3D() noexcept
{
    return m_primitiveRenderer3D;
}

const PrimitiveRenderer3D &RenderContext::GetPrimitiveRenderer3D() const noexcept
{
    return m_primitiveRenderer3D;
}

SceneRenderer3D &RenderContext::GetSceneRenderer3D() noexcept
{
    return m_sceneRenderer3D;
}

const SceneRenderer3D &RenderContext::GetSceneRenderer3D() const noexcept
{
    return m_sceneRenderer3D;
}

ModelRenderer &RenderContext::GetModelRenderer() noexcept
{
    return m_modelRenderer;
}

const ModelRenderer &RenderContext::GetModelRenderer() const noexcept
{
    return m_modelRenderer;
}

DebugDrawQueue &RenderContext::GetDebugDraw() noexcept
{
    return m_debugDraw;
}

const DebugDrawQueue &RenderContext::GetDebugDraw() const noexcept
{
    return m_debugDraw;
}

FrameRenderer &RenderContext::GetFrameRenderer() noexcept
{
    return m_frameRenderer;
}

const FrameRenderer &RenderContext::GetFrameRenderer() const noexcept
{
    return m_frameRenderer;
}

DeferredFrameResources &RenderContext::GetDeferredFrameResources() noexcept
{
    return m_deferredFrameResources;
}

const DeferredFrameResources &RenderContext::GetDeferredFrameResources() const noexcept
{
    return m_deferredFrameResources;
}

FrameRenderResources &RenderContext::GetFrameRenderResources() noexcept
{
    return m_frameRenderResources;
}

const FrameRenderResources &RenderContext::GetFrameRenderResources() const noexcept
{
    return m_frameRenderResources;
}

FrameRenderPipeline &RenderContext::GetFrameRenderPipeline() noexcept
{
    return m_frameRenderPipeline;
}

const FrameRenderPipeline &RenderContext::GetFrameRenderPipeline() const noexcept
{
    return m_frameRenderPipeline;
}

GpuParticleSystem &RenderContext::GetGpuParticleSystem() noexcept
{
    return m_gpuParticleSystem;
}

const GpuParticleSystem &RenderContext::GetGpuParticleSystem() const noexcept
{
    return m_gpuParticleSystem;
}

void RenderContext::BuildDefaultForwardRenderPipeline()
{
    if (m_graphics == nullptr)
    {
        return;
    }

    m_frameRenderPipeline.ClearPasses();
    m_frameRenderPipeline.AddPass(std::make_unique<ShadowDepthRenderPass>());
    m_frameRenderPipeline.AddPass(std::make_unique<MainGeometryBindRenderPass>());
    m_frameRenderPipeline.AddPass(std::make_unique<GameRenderPass>());
    m_frameRenderPipeline.AddPass(std::make_unique<ParticlesRenderPass>());
    m_frameRenderPipeline.AddPass(std::make_unique<DebugOverlayRenderPass>());
    m_frameRenderPipeline.AddPass(std::make_unique<UserInterfaceRenderPass>());
    m_frameRenderPipeline.Initialize(*m_graphics, m_frameRenderResources);
}

void RenderContext::BuildDefaultDeferredRenderPipeline()
{
    if (m_graphics == nullptr)
    {
        return;
    }

    m_frameRenderPipeline.ClearPasses();
    m_frameRenderPipeline.AddPass(std::make_unique<ShadowDepthRenderPass>());
    m_frameRenderPipeline.AddPass(std::make_unique<DeferredGeometryRenderPass>());
    m_frameRenderPipeline.AddPass(std::make_unique<GameRenderPass>());
    m_frameRenderPipeline.AddPass(std::make_unique<DeferredLightingRenderPass>());
    m_frameRenderPipeline.AddPass(std::make_unique<DeferredCompositeRenderPass>());
    m_frameRenderPipeline.AddPass(std::make_unique<GBufferDebugRenderPass>());
    m_frameRenderPipeline.AddPass(std::make_unique<ParticlesRenderPass>());
    m_frameRenderPipeline.AddPass(std::make_unique<DebugOverlayRenderPass>());
    m_frameRenderPipeline.AddPass(std::make_unique<UserInterfaceRenderPass>());
    m_frameRenderPipeline.Initialize(*m_graphics, m_frameRenderResources);
}

void RenderContext::SetRenderMode(const RenderMode renderMode)
{
    m_renderMode = renderMode;
    if (m_renderMode == RenderMode::Deferred)
    {
        BuildDefaultDeferredRenderPipeline();
        return;
    }

    BuildDefaultForwardRenderPipeline();
}

RenderMode RenderContext::GetRenderMode() const noexcept
{
    return m_renderMode;
}

bool RenderContext::IsDeferredRenderingEnabled() const noexcept
{
    return m_renderMode == RenderMode::Deferred;
}

RenderPassKind RenderContext::GetActiveRenderPassKind() const noexcept
{
    return m_frameRenderer.GetActivePass();
}

bool RenderContext::ShouldBindMainRenderTargetsForDraw() const noexcept
{
    if (!IsDeferredRenderingEnabled())
    {
        return true;
    }

    return GetActiveRenderPassKind() != RenderPassKind::DeferredGeometry;
}

void RenderContext::SetGameRenderCallbackForUserInterfacePassEnabled(const bool enabled) noexcept
{
    m_executeGameRenderCallbackDuringUserInterfacePass = enabled;
}

bool RenderContext::ShouldExecuteGameRenderCallbackDuringUserInterfacePass() const noexcept
{
    return m_executeGameRenderCallbackDuringUserInterfacePass;
}

void RenderContext::SetGBufferDebugVisualizationEnabled(const bool enabled) noexcept
{
    m_gBufferDebugVisualizationEnabled = enabled;
}

bool RenderContext::IsGBufferDebugVisualizationEnabled() const noexcept
{
    return m_gBufferDebugVisualizationEnabled;
}

void RenderContext::SetFrameGameplaySceneAndCamera(
    Scene *const gameplayScene,
    Camera *const activeCamera
) noexcept
{
    m_frameGameplaySceneForPipeline = gameplayScene;
    m_frameActiveCameraForPipeline = activeCamera;
}

void RenderContext::ExecuteFramePipeline(
    Camera *const activeCamera,
    Scene *const gameplayScene,
    std::function<void()> gameRenderCallback,
    const float frameDeltaTimeSeconds
)
{
    Camera *const effectiveCamera =
        activeCamera != nullptr ? activeCamera : m_frameActiveCameraForPipeline;
    Scene *const effectiveScene =
        gameplayScene != nullptr ? gameplayScene : m_frameGameplaySceneForPipeline;

    FramePassRenderContext framePassRenderContext(
        *this,
        m_frameRenderResources,
        effectiveCamera,
        effectiveScene,
        std::move(gameRenderCallback),
        static_cast<float>(m_frameRenderResources.GetBackbufferWidth()),
        static_cast<float>(m_frameRenderResources.GetBackbufferHeight()),
        frameDeltaTimeSeconds
    );
    m_frameRenderPipeline.ExecuteFrame(framePassRenderContext);
}
