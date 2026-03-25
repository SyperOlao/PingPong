#ifndef PINGPONG_SHADOWDEPTHPASSRASTERIZERSTATE_H
#define PINGPONG_SHADOWDEPTHPASSRASTERIZERSTATE_H

#include <d3d11.h>
#include <wrl/client.h>

class ShadowDepthPassRasterizerState final
{
public:
    void Create(
        ID3D11Device *device,
        D3D11_CULL_MODE cullMode,
        int depthBias,
        float slopeScaledDepthBias,
        float depthBiasClamp
    );

    void Destroy() noexcept;

    [[nodiscard]] ID3D11RasterizerState *Get() const noexcept;

private:
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;
};

#endif
