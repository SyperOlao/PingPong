#ifndef PINGPONG_SHADERCONSTANTS3D_H
#define PINGPONG_SHADERCONSTANTS3D_H

#include <DirectXMath.h>

#include <SimpleMath.h>

#include <cstddef>
#include <cstdint>

constexpr std::uint32_t kMaximumSceneLights = 8u;

enum class LightKindGpu : std::uint32_t
{
    Directional = 0u,
    Point = 1u,
    Spot = 2u
};

struct alignas(16) LightGpu final
{
    DirectX::XMFLOAT4 Position{};
    DirectX::XMFLOAT4 Direction{};
    DirectX::XMFLOAT4 Color{};
    DirectX::XMFLOAT4 Parameters{};
    DirectX::XMFLOAT4 Extension{};
};

struct alignas(16) CameraGpuConstants final
{
    DirectX::SimpleMath::Matrix View{};
    DirectX::SimpleMath::Matrix Projection{};
    DirectX::SimpleMath::Matrix ViewProjection{};
    DirectX::XMFLOAT4 WorldPosition{};
};

struct alignas(16) ObjectGpuConstants final
{
    DirectX::SimpleMath::Matrix World{};
    DirectX::SimpleMath::Matrix WorldInverseTranspose{};
};

struct alignas(16) MaterialGpuConstants final
{
    DirectX::XMFLOAT4 BaseColor{};
    DirectX::XMFLOAT4 SpecularColorAndPower{};
    DirectX::XMFLOAT4 EmissiveAndAmbient{};
};

struct alignas(16) LightsGpuConstants final
{
    std::uint32_t TotalLightCount{0u};
    std::uint32_t Padding0{0u};
    std::uint32_t Padding1{0u};
    std::uint32_t Padding2{0u};
    LightGpu Lights[kMaximumSceneLights]{};
};

static_assert(sizeof(CameraGpuConstants) % 16u == 0u);
static_assert(sizeof(ObjectGpuConstants) % 16u == 0u);
static_assert(sizeof(MaterialGpuConstants) % 16u == 0u);
static_assert(sizeof(LightGpu) % 16u == 0u);
static_assert(sizeof(LightsGpuConstants) % 16u == 0u);

#endif
