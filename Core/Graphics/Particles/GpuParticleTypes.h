#ifndef PINGPONG_GPUPARTICLETYPES_H
#define PINGPONG_GPUPARTICLETYPES_H

#include <DirectXMath.h>
#include <SimpleMath.h>

#include <cstdint>

struct GpuParticleEmitterDesc final
{
    bool Enabled{false};
    float SpawnRateParticlesPerSecond{180.0f};
    DirectX::SimpleMath::Vector3 SpawnVolumeCenter{0.0f, 18.0f, 0.0f};
    DirectX::SimpleMath::Vector3 SpawnVolumeHalfExtents{24.0f, 4.0f, 24.0f};
    DirectX::SimpleMath::Vector3 InitialVelocityMin{-0.8f, -7.5f, -0.8f};
    DirectX::SimpleMath::Vector3 InitialVelocityMax{0.8f, -4.0f, 0.8f};
    DirectX::SimpleMath::Vector3 Gravity{0.0f, -8.0f, 0.0f};
    DirectX::SimpleMath::Color Color{0.42f, 0.82f, 1.0f, 0.72f};
    float LifetimeSeconds{4.5f};
    float StartSize{0.42f};
    float CollisionDamping{0.42f};
    float CollisionDepthBias{0.0016f};
    bool DepthCollisionEnabled{true};
    float DrawSizeScale{1.0f};
    float DrawAlphaScale{1.0f};
};

struct alignas(16) GpuParticleData final
{
    DirectX::XMFLOAT4 PositionLife{};
    DirectX::XMFLOAT4 VelocitySize{};
    DirectX::XMFLOAT4 Color{};
};

struct alignas(16) GpuParticleSortKey final
{
    float Key{0.0f};
    std::uint32_t Index{0u};
    std::uint32_t Alive{0u};
    std::uint32_t Padding{0u};
};

struct alignas(16) GpuParticleUpdateConstants final
{
    DirectX::SimpleMath::Matrix ViewProjection{};
    DirectX::SimpleMath::Matrix InverseProjection{};
    DirectX::SimpleMath::Matrix InverseView{};
    DirectX::XMFLOAT4 EmitterCenterAndDeltaTime{};
    DirectX::XMFLOAT4 EmitterExtentsAndLifetime{};
    DirectX::XMFLOAT4 VelocityMinAndStartSize{};
    DirectX::XMFLOAT4 VelocityMaxAndCollisionDamping{};
    DirectX::XMFLOAT4 GravityAndCollisionBias{};
    DirectX::XMFLOAT4 ParticleColor{};
    DirectX::XMFLOAT4 CameraWorldPosition{};
    DirectX::XMFLOAT4 DepthTextureSizeAndCollisionEnabled{};
    std::uint32_t MaxParticles{0u};
    std::uint32_t SpawnStartIndex{0u};
    std::uint32_t SpawnCount{0u};
    std::uint32_t FrameIndex{0u};
};

struct alignas(16) GpuParticleSortConstants final
{
    std::uint32_t SortLevel{0u};
    std::uint32_t CompareDistance{0u};
    std::uint32_t ElementCount{0u};
    std::uint32_t Padding{0u};
};

struct alignas(16) GpuParticleDrawConstants final
{
    DirectX::SimpleMath::Matrix ViewProjection{};
    DirectX::XMFLOAT4 CameraRightAndSizeScale{};
    DirectX::XMFLOAT4 CameraUpAndAlphaScale{};
    DirectX::XMFLOAT4 ScreenInverseSizeAndDepthBias{};
};

static_assert(sizeof(GpuParticleData) % 16u == 0u);
static_assert(sizeof(GpuParticleSortKey) % 16u == 0u);
static_assert(sizeof(GpuParticleUpdateConstants) % 16u == 0u);
static_assert(sizeof(GpuParticleSortConstants) % 16u == 0u);
static_assert(sizeof(GpuParticleDrawConstants) % 16u == 0u);

#endif
