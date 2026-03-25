#ifndef PINGPONG_SHADOWSHADERCONSTANTS3D_H
#define PINGPONG_SHADOWSHADERCONSTANTS3D_H

#include <DirectXMath.h>

#include <SimpleMath.h>

#include <cstdint>

constexpr std::uint32_t kMaximumShadowCascades = 4u;

struct alignas(16) ShadowPassLightConstantsGpu final
{
    DirectX::SimpleMath::Matrix LightViewProjection{};
    DirectX::XMFLOAT4 ShadowBiasAndTexelSize{};
};

struct alignas(16) ShadowCascadeConstantsGpu final
{
    DirectX::SimpleMath::Matrix LightViewProjection[kMaximumShadowCascades]{};
    DirectX::XMFLOAT4 CascadeSplits{};
    std::uint32_t CascadeCount{0u};
    std::uint32_t Padding0{0u};
    std::uint32_t Padding1{0u};
    std::uint32_t Padding2{0u};
};

static_assert(sizeof(ShadowPassLightConstantsGpu) % 16u == 0u);
static_assert(sizeof(ShadowCascadeConstantsGpu) % 16u == 0u);

#endif
