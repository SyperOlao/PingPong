#include "Game/Katamari/KatamariSceneLighting.h"

#include "Core/Graphics/Rendering/Lighting/LightTypes3D.h"

#include <array>
#include <algorithm>

using DirectX::SimpleMath::Color;
using DirectX::SimpleMath::Vector3;

SceneLightingDescriptor3D CreateKatamariSceneLighting(const KatamariGameConfig &config)
{
    SceneLightingDescriptor3D lighting = SceneLighting3DCreateDefaultOutdoor();
    if (!lighting.DirectionalLights.empty())
    {
        lighting.DirectionalLights[0].Direction = Vector3(-0.55f, -0.78f, -0.35f);
        lighting.DirectionalLights[0].Intensity = 1.15f;
        lighting.DirectionalLights[0].LightColor = Color(0.98f, 0.96f, 0.9f, 1.0f);
    }

    const float halfExtent = config.PlayfieldHalfExtent;
    struct EmissiveObstacleLightDefinition final
    {
        Vector3 Position{};
        Vector3 Scale{1.0f, 1.0f, 1.0f};
        Color LightColor{1.0f, 1.0f, 1.0f, 1.0f};
    };

    const std::array<EmissiveObstacleLightDefinition, 6> obstacleLights =
    {{
        {Vector3(-halfExtent * 0.55f, 0.0f, -halfExtent * 0.28f), Vector3(8.0f, 8.0f, 8.0f), Color(0.1f, 0.95f, 0.95f, 1.0f)},
        {Vector3(-halfExtent * 0.24f, 0.0f, halfExtent * 0.48f), Vector3(10.0f, 12.0f, 6.0f), Color(1.0f, 0.28f, 0.72f, 1.0f)},
        {Vector3(halfExtent * 0.18f, 0.0f, -halfExtent * 0.5f), Vector3(7.0f, 11.0f, 7.0f), Color(0.55f, 1.0f, 0.2f, 1.0f)},
        {Vector3(halfExtent * 0.56f, 0.0f, halfExtent * 0.14f), Vector3(12.0f, 9.0f, 8.0f), Color(1.0f, 0.72f, 0.16f, 1.0f)},
        {Vector3(-halfExtent * 0.08f, 0.0f, -halfExtent * 0.08f), Vector3(6.0f, 14.0f, 6.0f), Color(0.45f, 0.56f, 1.0f, 1.0f)},
        {Vector3(halfExtent * 0.36f, 0.0f, halfExtent * 0.55f), Vector3(8.0f, 10.0f, 10.0f), Color(0.96f, 0.2f, 1.0f, 1.0f)}
    }};

    for (const EmissiveObstacleLightDefinition &definition : obstacleLights)
    {
        const float largestScale = (std::max)(definition.Scale.x, (std::max)(definition.Scale.y, definition.Scale.z));
        PointLight3D pointLight{};
        pointLight.Position = Vector3(
            definition.Position.x,
            definition.Scale.y * 0.62f + 1.5f,
            definition.Position.z
        );
        pointLight.Range = (std::max)(largestScale * 5.0f, 42.0f);
        pointLight.Intensity = 0.4f;
        pointLight.LightColor = definition.LightColor;
        lighting.PointLights.push_back(pointLight);
    }

    SpotLight3D stageSpot{};
    stageSpot.Position = Vector3(0.0f, 62.0f, -46.0f);
    stageSpot.Direction = Vector3(0.0f, -0.82f, 0.58f);
    stageSpot.Range = 135.0f;
    stageSpot.InnerConeAngleRadians = 0.42f;
    stageSpot.OuterConeAngleRadians = 0.78f;
    stageSpot.Intensity = 0.62f;
    stageSpot.LightColor = Color(1.0f, 0.94f, 0.82f, 1.0f);
    lighting.SpotLights.push_back(stageSpot);

    return lighting;
}

GpuParticleEmitterDesc CreateKatamariParticleEmitterDesc(const KatamariGameConfig &config)
{
    const float halfExtent = config.PlayfieldHalfExtent;

    GpuParticleEmitterDesc particles{};
    particles.Enabled = true;
    particles.SpawnRateParticlesPerSecond = config.ParticleSpawnRateParticlesPerSecond;
    particles.SpawnVolumeCenter = Vector3(0.0f, config.ParticleSpawnVolumeCenterHeight, 0.0f);
    particles.SpawnVolumeHalfExtents = Vector3(
        halfExtent * config.ParticleSpawnVolumeHalfExtentScale,
        config.ParticleSpawnVolumeHalfHeight,
        halfExtent * config.ParticleSpawnVolumeHalfExtentScale
    );
    particles.InitialVelocityMin = Vector3(-1.05f, -8.8f, -1.05f);
    particles.InitialVelocityMax = Vector3(1.05f, -4.9f, 1.05f);
    particles.Gravity = Vector3(0.0f, -5.2f, 0.0f);
    particles.Color = Color(0.44f, 0.82f, 1.0f, 0.58f);
    particles.LifetimeSeconds = 5.4f;
    particles.StartSize = 0.34f;
    particles.CollisionDamping = 0.34f;
    particles.CollisionDepthBias = 0.0018f;
    particles.DepthCollisionEnabled = true;
    particles.DrawSizeScale = 1.0f;
    particles.DrawAlphaScale = 0.88f;
    return particles;
}
