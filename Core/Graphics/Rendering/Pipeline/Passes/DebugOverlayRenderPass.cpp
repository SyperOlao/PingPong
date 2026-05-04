#include "Core/Graphics/Rendering/Pipeline/Passes/DebugOverlayRenderPass.h"

#include "Core/Graphics/Camera.h"
#include "Core/Graphics/GraphicsDevice.h"
#include "Core/Graphics/Rendering/Pipeline/FramePassRenderContext.h"
#include "Core/Graphics/Rendering/Pipeline/FrameRenderResources.h"
#include "Core/Graphics/Rendering/RenderContext.h"

void DebugOverlayRenderPass::Initialize(GraphicsDevice &, FrameRenderResources &)
{
}

void DebugOverlayRenderPass::Shutdown()
{
}

void DebugOverlayRenderPass::Resize(int, int)
{
}

void DebugOverlayRenderPass::Execute(FramePassRenderContext &framePassRenderContext)
{
    Camera *const camera = framePassRenderContext.GetActiveCamera();
    if (camera == nullptr)
    {
        return;
    }

    const float aspectRatio = framePassRenderContext.GetViewportAspectRatio();
    const DirectX::SimpleMath::Matrix viewMatrix = camera->GetViewMatrix();
    const DirectX::SimpleMath::Matrix projectionMatrix = camera->GetProjectionMatrix(aspectRatio);

    framePassRenderContext.GetRenderContext().GetFrameRenderer().EnterPass(RenderPassKind::DebugOverlay);
    framePassRenderContext.GetRenderContext().GetDebugDraw().Flush(
        framePassRenderContext.GetRenderContext().GetPrimitiveRenderer3D(),
        viewMatrix,
        projectionMatrix
    );
    framePassRenderContext.GetRenderContext().GetFrameRenderer().LeavePass();
}
