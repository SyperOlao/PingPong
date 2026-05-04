#include "Core/Graphics/Rendering/Pipeline/Passes/DeferredCompositeRenderPass.h"
#include "Core/Graphics/D3d11Helpers.h"
#include "Core/Graphics/GraphicsDevice.h"
#include "Core/Graphics/Rendering/Deferred/DeferredFrameResources.h"
#include "Core/Graphics/Rendering/Pipeline/FramePassRenderContext.h"
#include "Core/Graphics/Rendering/Pipeline/FrameRenderResources.h"
#include "Core/Graphics/ShaderCompiler.h"

void DeferredCompositeRenderPass::Initialize(GraphicsDevice &graphicsDevice, FrameRenderResources &)
{
    ID3D11Device *const device = graphicsDevice.GetDevice();
    if (device == nullptr)
    {
        return;
    }

    const auto vertexShaderBlob = ShaderCompiler::CompileFromFile("Core/Shaders/DeferredComposite3D.hlsl", "VSMain", "vs_5_0");
    const auto pixelShaderBlob = ShaderCompiler::CompileFromFile("Core/Shaders/DeferredComposite3D.hlsl", "PSMain", "ps_5_0");
    D3d11Helpers::ThrowIfFailed(
        device->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), nullptr, VertexShader.GetAddressOf()),
        "DeferredCompositeRenderPass failed to create vertex shader."
    );
    D3d11Helpers::ThrowIfFailed(
        device->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), nullptr, PixelShader.GetAddressOf()),
        "DeferredCompositeRenderPass failed to create pixel shader."
    );

    D3D11_SAMPLER_DESC samplerDescription{};
    samplerDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDescription.MinLOD = 0.0f;
    samplerDescription.MaxLOD = D3D11_FLOAT32_MAX;
    D3d11Helpers::ThrowIfFailed(
        device->CreateSamplerState(&samplerDescription, SamplerState.GetAddressOf()),
        "DeferredCompositeRenderPass failed to create sampler state."
    );
}

void DeferredCompositeRenderPass::Shutdown()
{
    SamplerState.Reset();
    PixelShader.Reset();
    VertexShader.Reset();
}

void DeferredCompositeRenderPass::Resize(int, int)
{
}

void DeferredCompositeRenderPass::Execute(FramePassRenderContext &framePassRenderContext)
{
    DeferredFrameResources *const deferredFrameResources = framePassRenderContext.GetFrameRenderResources().GetDeferredFrameResources();
    if (deferredFrameResources == nullptr || !deferredFrameResources->IsCreated())
    {
        return;
    }

    GraphicsDevice &graphicsDevice = framePassRenderContext.GetGraphicsDevice();
    graphicsDevice.BindSingleRenderTarget(graphicsDevice.GetMainRenderTargetView());
    graphicsDevice.SetMainViewport();

    ID3D11DeviceContext *const deviceContext = graphicsDevice.GetImmediateContext();
    if (deviceContext == nullptr || VertexShader == nullptr || PixelShader == nullptr)
    {
        return;
    }

    ID3D11ShaderResourceView *const lightShaderResourceView = deferredFrameResources->GetLightAccumulationTarget().GetShaderResourceView();
    ID3D11ShaderResourceView *const shaderResourceViews[] = {lightShaderResourceView};
    deviceContext->PSSetShaderResources(0, 1, shaderResourceViews);

    ID3D11SamplerState *const samplers[] = {SamplerState.Get()};
    deviceContext->PSSetSamplers(0, 1, samplers);
    deviceContext->IASetInputLayout(nullptr);
    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    deviceContext->VSSetShader(VertexShader.Get(), nullptr, 0);
    deviceContext->PSSetShader(PixelShader.Get(), nullptr, 0);
    deviceContext->Draw(3, 0);

    ID3D11ShaderResourceView *const nullShaderResourceViews[] = {nullptr};
    deviceContext->PSSetShaderResources(0, 1, nullShaderResourceViews);
}
