#include "Core/Graphics/Rendering/Pipeline/Passes/ParticlesRenderPass.h"

#include "Core/Graphics/Camera.h"
#include "Core/Graphics/GraphicsDevice.h"
#include "Core/Graphics/Particles/GpuParticleSystem.h"
#include "Core/Graphics/Rendering/Deferred/DeferredFrameResources.h"
#include "Core/Graphics/Rendering/Deferred/GBufferResources.h"
#include "Core/Graphics/Rendering/RenderContext.h"
#include "Core/Graphics/Rendering/Shadows/DepthRenderTarget2D.h"
#include "Core/Graphics/Rendering/Pipeline/FramePassRenderContext.h"
#include "Core/Graphics/Rendering/Pipeline/FrameRenderResources.h"

void ParticlesRenderPass::Initialize(GraphicsDevice &, FrameRenderResources &)
{
}

void ParticlesRenderPass::Shutdown()
{
}

void ParticlesRenderPass::Resize(int, int)
{
}

void ParticlesRenderPass::Execute(FramePassRenderContext &framePassRenderContext)
{
    Camera *const activeCamera = framePassRenderContext.GetActiveCamera();
    if (activeCamera == nullptr)
    {
        return;
    }

    RenderContext &renderContext = framePassRenderContext.GetRenderContext();
    GpuParticleSystem &particleSystem = renderContext.GetGpuParticleSystem();
    if (!particleSystem.IsInitialized() || !particleSystem.GetEmitterDesc().Enabled)
    {
        return;
    }

    GraphicsDevice &graphicsDevice = framePassRenderContext.GetGraphicsDevice();

    ID3D11ShaderResourceView *sceneDepthShaderResourceView = nullptr;
    DeferredFrameResources *const deferredFrameResources =
        framePassRenderContext.GetFrameRenderResources().GetDeferredFrameResources();
    if (deferredFrameResources != nullptr && deferredFrameResources->IsCreated())
    {
        sceneDepthShaderResourceView =
            deferredFrameResources->GetGBufferResources().GetSceneDepthTarget().GetShaderResourceView();
    }

    renderContext.GetFrameRenderer().EnterPass(RenderPassKind::Particles);
    graphicsDevice.BindMainRenderTargets();
    graphicsDevice.SetMainViewport();

    particleSystem.SimulateAndRender(
        graphicsDevice,
        *activeCamera,
        framePassRenderContext.GetViewportAspectRatio(),
        framePassRenderContext.GetFrameDeltaTimeSeconds(),
        sceneDepthShaderResourceView
    );

    renderContext.GetFrameRenderer().LeavePass();
}
