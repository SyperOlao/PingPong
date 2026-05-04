//
// Created by SyperOlao on 25.03.2026.
//

#ifndef PINGPONG_RENDERCONTEXT_H
#define PINGPONG_RENDERCONTEXT_H

#include "PrimitiveRenderer3D.h"
#include "ShapeRenderer2D.h"

#include "Core/Graphics/Particles/GpuParticleSystem.h"
#include "Core/Graphics/Rendering/Deferred/DeferredFrameResources.h"
#include "Core/Graphics/Debug/DebugDrawQueue.h"
#include "Core/Graphics/ModelRenderer.h"
#include "Core/Graphics/Rendering/FrameRenderer.h"
#include "Core/Graphics/Rendering/Pipeline/FrameRenderPipeline.h"
#include "Core/Graphics/Rendering/Pipeline/FrameRenderResources.h"
#include "Core/Graphics/Rendering/FrameRenderTypes.h"
#include "Core/Graphics/Rendering/Renderables/SceneRenderer3D.h"
#include "Core/Graphics/Rendering/Shadows/DirectionalShadowResources.h"

#include <d3d11.h>
#include <functional>

class Camera;
class GraphicsDevice;
class Scene;

enum class RenderMode : std::uint8_t
{
    Forward = 0,
    Deferred
};

class RenderContext final {
public:
    RenderContext() = default;

    ~RenderContext();

    RenderContext(const RenderContext &) = delete;

    RenderContext &operator=(const RenderContext &) = delete;

    RenderContext(RenderContext &&) = delete;

    RenderContext &operator=(RenderContext &&) = delete;

    void Initialize(GraphicsDevice &graphics);

    void BeginRenderFrame();

    void ResizeDeferredResources();

    void SetDirectionalShadowSamplingParameters(
        float constantDepthBias,
        float slopeScaledDepthBias,
        float normalOffsetWorldUnits,
        float pcfRadiusTexels
    ) noexcept;

    void PrepareDirectionalShadowPass(Scene &scene, Camera &camera);

    void InvalidateDirectionalShadowPass();

    void BindForwardPhongShadowRegisters(ID3D11DeviceContext *deviceContext) const;

    void UnbindForwardPhongShadowShaderResource(ID3D11DeviceContext *deviceContext) const;

    void BindDirectionalShadowRegisters(
        ID3D11DeviceContext *deviceContext,
        UINT shadowMapShaderResourceSlot,
        UINT comparisonSamplerSlot
    ) const;

    void UnbindDirectionalShadowShaderResource(
        ID3D11DeviceContext *deviceContext,
        UINT shadowMapShaderResourceSlot
    ) const;

    [[nodiscard]] ShapeRenderer2D &GetShapeRenderer2D() noexcept;

    [[nodiscard]] const ShapeRenderer2D &GetShapeRenderer2D() const noexcept;

    [[nodiscard]] PrimitiveRenderer3D &GetPrimitiveRenderer3D() noexcept;

    [[nodiscard]] const PrimitiveRenderer3D &GetPrimitiveRenderer3D() const noexcept;

    [[nodiscard]] SceneRenderer3D &GetSceneRenderer3D() noexcept;

    [[nodiscard]] const SceneRenderer3D &GetSceneRenderer3D() const noexcept;

    [[nodiscard]] ModelRenderer &GetModelRenderer() noexcept;

    [[nodiscard]] const ModelRenderer &GetModelRenderer() const noexcept;

    [[nodiscard]] DebugDrawQueue &GetDebugDraw() noexcept;

    [[nodiscard]] const DebugDrawQueue &GetDebugDraw() const noexcept;

    [[nodiscard]] FrameRenderer &GetFrameRenderer() noexcept;

    [[nodiscard]] const FrameRenderer &GetFrameRenderer() const noexcept;

    [[nodiscard]] DeferredFrameResources &GetDeferredFrameResources() noexcept;

    [[nodiscard]] const DeferredFrameResources &GetDeferredFrameResources() const noexcept;

    [[nodiscard]] FrameRenderResources &GetFrameRenderResources() noexcept;

    [[nodiscard]] const FrameRenderResources &GetFrameRenderResources() const noexcept;

    [[nodiscard]] FrameRenderPipeline &GetFrameRenderPipeline() noexcept;

    [[nodiscard]] const FrameRenderPipeline &GetFrameRenderPipeline() const noexcept;

    [[nodiscard]] GpuParticleSystem &GetGpuParticleSystem() noexcept;

    [[nodiscard]] const GpuParticleSystem &GetGpuParticleSystem() const noexcept;

    void BuildDefaultForwardRenderPipeline();
    void BuildDefaultDeferredRenderPipeline();

    void SetRenderMode(RenderMode renderMode);
    [[nodiscard]] RenderMode GetRenderMode() const noexcept;
    [[nodiscard]] bool IsDeferredRenderingEnabled() const noexcept;
    [[nodiscard]] RenderPassKind GetActiveRenderPassKind() const noexcept;
    [[nodiscard]] bool ShouldBindMainRenderTargetsForDraw() const noexcept;
    void SetGameRenderCallbackForUserInterfacePassEnabled(bool enabled) noexcept;
    [[nodiscard]] bool ShouldExecuteGameRenderCallbackDuringUserInterfacePass() const noexcept;
    void SetGBufferDebugVisualizationEnabled(bool enabled) noexcept;
    [[nodiscard]] bool IsGBufferDebugVisualizationEnabled() const noexcept;

    void ExecuteFramePipeline(
        Camera *activeCamera,
        Scene *gameplayScene,
        std::function<void()> gameRenderCallback,
        float frameDeltaTimeSeconds
    );

    void SetFrameGameplaySceneAndCamera(Scene *gameplayScene, Camera *activeCamera) noexcept;

private:
    GraphicsDevice *m_graphics{nullptr};
    ShapeRenderer2D m_shapeRenderer2D;
    PrimitiveRenderer3D m_primitiveRenderer3D;
    ModelRenderer m_modelRenderer{};
    SceneRenderer3D m_sceneRenderer3D{};
    DebugDrawQueue m_debugDraw{};
    FrameRenderer m_frameRenderer{};
    DeferredFrameResources m_deferredFrameResources{};
    FrameRenderResources m_frameRenderResources{};
    FrameRenderPipeline m_frameRenderPipeline{};
    DirectionalShadowResources m_directionalShadowResources{};
    GpuParticleSystem m_gpuParticleSystem{};
    RenderMode m_renderMode{RenderMode::Forward};
    bool m_executeGameRenderCallbackDuringUserInterfacePass{false};
    bool m_gBufferDebugVisualizationEnabled{false};
    bool m_directionalShadowPassCompletedThisFrame{false};
    Scene *m_frameGameplaySceneForPipeline{nullptr};
    Camera *m_frameActiveCameraForPipeline{nullptr};
};

#endif
