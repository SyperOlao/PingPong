#include "Core/Graphics/Rendering/Pipeline/Passes/GBufferDebugRenderPass.h"

#include "Core/Graphics/D3d11Helpers.h"
#include "Core/Graphics/GraphicsDevice.h"
#include "Core/Graphics/Rendering/Deferred/DeferredFrameResources.h"
#include "Core/Graphics/Rendering/Deferred/GBufferResources.h"
#include "Core/Graphics/Rendering/Pipeline/FramePassRenderContext.h"
#include "Core/Graphics/Rendering/Pipeline/FrameRenderResources.h"
#include "Core/Graphics/Rendering/RenderContext.h"
#include "Core/Graphics/ShaderCompiler.h"

void GBufferDebugRenderPass::Initialize(GraphicsDevice &graphicsDevice, FrameRenderResources &)
{
    ID3D11Device *const device = graphicsDevice.GetDevice();
    if (device == nullptr)
    {
        return;
    }

    const auto vertexShaderBlob = ShaderCompiler::CompileFromFile("Core/Shaders/GBufferDebugComposite3D.hlsl", "VSMain", "vs_5_0");
    const auto pixelShaderBlob = ShaderCompiler::CompileFromFile("Core/Shaders/GBufferDebugComposite3D.hlsl", "PSMain", "ps_5_0");
    D3d11Helpers::ThrowIfFailed(
        device->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), nullptr, VertexShader.GetAddressOf()),
        "GBufferDebugRenderPass failed to create vertex shader."
    );
    D3d11Helpers::ThrowIfFailed(
        device->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), nullptr, PixelShader.GetAddressOf()),
        "GBufferDebugRenderPass failed to create pixel shader."
    );

    D3D11_SAMPLER_DESC samplerDescription{};
    samplerDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDescription.MinLOD = 0.0f;
    samplerDescription.MaxLOD = D3D11_FLOAT32_MAX;
    D3d11Helpers::ThrowIfFailed(
        device->CreateSamplerState(&samplerDescription, SamplerState.GetAddressOf()),
        "GBufferDebugRenderPass failed to create sampler state."
    );
}

void GBufferDebugRenderPass::Shutdown()
{
    SamplerState.Reset();
    PixelShader.Reset();
    VertexShader.Reset();
}

void GBufferDebugRenderPass::Resize(int, int)
{
}

void GBufferDebugRenderPass::Execute(FramePassRenderContext &framePassRenderContext)
{
    if (!framePassRenderContext.GetRenderContext().IsGBufferDebugVisualizationEnabled())
    {
        return;
    }

    DeferredFrameResources *const deferredFrameResources = framePassRenderContext.GetFrameRenderResources().GetDeferredFrameResources();
    if (deferredFrameResources == nullptr || !deferredFrameResources->IsCreated())
    {
        return;
    }

    GraphicsDevice &graphicsDevice = framePassRenderContext.GetGraphicsDevice();
    ID3D11DeviceContext *const deviceContext = graphicsDevice.GetImmediateContext();
    if (deviceContext == nullptr || VertexShader == nullptr || PixelShader == nullptr || SamplerState == nullptr)
    {
        return;
    }

    graphicsDevice.BindSingleRenderTarget(graphicsDevice.GetMainRenderTargetView());
    graphicsDevice.SetMainViewport();
    graphicsDevice.ClearMainColor(Color(0.015f, 0.018f, 0.024f, 1.0f));
    framePassRenderContext.GetRenderContext().GetFrameRenderer().EnterPass(RenderPassKind::GBufferDebug);

    GBufferResources &gBuffer = deferredFrameResources->GetGBufferResources();
    ID3D11ShaderResourceView *const shaderResourceViews[] =
    {
        gBuffer.GetAlbedoOcclusionTarget().GetShaderResourceView(),
        gBuffer.GetNormalTarget().GetShaderResourceView(),
        gBuffer.GetMaterialTarget().GetShaderResourceView(),
        gBuffer.GetEmissiveTarget().GetShaderResourceView(),
        gBuffer.GetSceneDepthTarget().GetShaderResourceView()
    };

    deviceContext->PSSetShaderResources(0, 5, shaderResourceViews);
    ID3D11SamplerState *const samplers[] = {SamplerState.Get()};
    deviceContext->PSSetSamplers(0, 1, samplers);
    deviceContext->IASetInputLayout(nullptr);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->VSSetShader(VertexShader.Get(), nullptr, 0);
    deviceContext->PSSetShader(PixelShader.Get(), nullptr, 0);
    deviceContext->Draw(3, 0);

    ID3D11ShaderResourceView *const nullShaderResourceViews[5] = {};
    deviceContext->PSSetShaderResources(0, 5, nullShaderResourceViews);
    framePassRenderContext.GetRenderContext().GetFrameRenderer().LeavePass();
}
