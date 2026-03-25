#ifndef PINGPONG_DEFERREDSHADERCONSTANTS3D_H
#define PINGPONG_DEFERREDSHADERCONSTANTS3D_H

#include <DirectXMath.h>

#include <SimpleMath.h>

#include <cstdint>

struct alignas(16) DeferredScreenConstantsGpu final
{
    DirectX::SimpleMath::Matrix InverseProjection{};
    DirectX::SimpleMath::Matrix InverseView{};
    DirectX::XMFLOAT4 CameraWorldPosition{};
    DirectX::XMFLOAT4 ScreenInverseWidthHeightAndFrameIndex{};
};

struct alignas(16) DeferredGBufferSamplingConstantsGpu final
{
    DirectX::XMFLOAT4 GBufferTextureResolutionAndPadding{};
    std::uint32_t GBufferWidthPixels{0u};
    std::uint32_t GBufferHeightPixels{0u};
    std::uint32_t Padding0{0u};
    std::uint32_t Padding1{0u};
};

static_assert(sizeof(DeferredScreenConstantsGpu) % 16u == 0u);
static_assert(sizeof(DeferredGBufferSamplingConstantsGpu) % 16u == 0u);

#endif
