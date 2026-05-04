#ifndef PINGPONG_GBUFFERDEBUGRENDERPASS_H
#define PINGPONG_GBUFFERDEBUGRENDERPASS_H

#include "Core/Graphics/Rendering/Pipeline/IRenderPass.h"

#include <wrl/client.h>

struct ID3D11PixelShader;
struct ID3D11SamplerState;
struct ID3D11VertexShader;

class GBufferDebugRenderPass final : public IRenderPass
{
public:
    void Initialize(GraphicsDevice &graphicsDevice, FrameRenderResources &frameRenderResources) override;
    void Shutdown() override;
    void Resize(int width, int height) override;
    void Execute(FramePassRenderContext &framePassRenderContext) override;

private:
    Microsoft::WRL::ComPtr<ID3D11VertexShader> VertexShader{};
    Microsoft::WRL::ComPtr<ID3D11PixelShader> PixelShader{};
    Microsoft::WRL::ComPtr<ID3D11SamplerState> SamplerState{};
};

#endif
