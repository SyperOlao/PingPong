#ifndef PINGPONG_SHADOWCOMPARISONSAMPLERSTATE_H
#define PINGPONG_SHADOWCOMPARISONSAMPLERSTATE_H

#include <d3d11.h>
#include <wrl/client.h>

class ShadowComparisonSamplerState final
{
public:
    void Create(ID3D11Device *device);

    void Destroy() noexcept;

    [[nodiscard]] ID3D11SamplerState *Get() const noexcept;

private:
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampler;
};

#endif
