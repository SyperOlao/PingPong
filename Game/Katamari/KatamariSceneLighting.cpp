#include "Game/Katamari/KatamariSceneLighting.h"

#include "Core/Graphics/Rendering/Lighting/LightTypes3D.h"

using DirectX::SimpleMath::Color;
using DirectX::SimpleMath::Vector3;

SceneLightingDescriptor3D CreateKatamariSceneLighting(const KatamariGameConfig &config)
{
    SceneLightingDescriptor3D lighting = SceneLighting3DCreateDefaultOutdoor();
    if (!lighting.DirectionalLights.empty())
    {
        lighting.DirectionalLights[0].Direction = Vector3(-0.22f, -1.0f, -0.28f);
        lighting.DirectionalLights[0].Intensity = 0.58f;
        lighting.DirectionalLights[0].LightColor = Color(0.78f, 0.84f, 1.0f, 1.0f);
    }

    PointLight3D fillLightNorth{};
    fillLightNorth.Position = Vector3(0.0f, 32.0f, 58.0f);
    fillLightNorth.Range = 130.0f;
    fillLightNorth.Intensity = 0.22f;
    fillLightNorth.LightColor = Color(1.0f, 0.94f, 0.82f, 1.0f);
    lighting.PointLights.push_back(fillLightNorth);

    PointLight3D fillLightSouth{};
    fillLightSouth.Position = Vector3(0.0f, 26.0f, -58.0f);
    fillLightSouth.Range = 130.0f;
    fillLightSouth.Intensity = 0.2f;
    fillLightSouth.LightColor = Color(0.82f, 0.9f, 1.0f, 1.0f);
    lighting.PointLights.push_back(fillLightSouth);

    PointLight3D fillLightOverhead{};
    fillLightOverhead.Position = Vector3(0.0f, 48.0f, 0.0f);
    fillLightOverhead.Range = 140.0f;
    fillLightOverhead.Intensity = 0.26f;
    fillLightOverhead.LightColor = Color(0.92f, 0.96f, 1.0f, 1.0f);
    lighting.PointLights.push_back(fillLightOverhead);

    const float halfExtent = config.PlayfieldHalfExtent;

    PointLight3D neonClusterWest{};
    neonClusterWest.Position = Vector3(-halfExtent * 0.55f, 10.0f, -halfExtent * 0.28f);
    neonClusterWest.Range = 38.0f;
    neonClusterWest.Intensity = 0.85f;
    neonClusterWest.LightColor = Color(0.1f, 0.95f, 0.95f, 1.0f);
    lighting.PointLights.push_back(neonClusterWest);

    PointLight3D neonClusterNorth{};
    neonClusterNorth.Position = Vector3(-halfExtent * 0.24f, 12.0f, halfExtent * 0.48f);
    neonClusterNorth.Range = 42.0f;
    neonClusterNorth.Intensity = 0.75f;
    neonClusterNorth.LightColor = Color(1.0f, 0.28f, 0.72f, 1.0f);
    lighting.PointLights.push_back(neonClusterNorth);

    PointLight3D neonClusterEast{};
    neonClusterEast.Position = Vector3(halfExtent * 0.56f, 10.0f, halfExtent * 0.14f);
    neonClusterEast.Range = 44.0f;
    neonClusterEast.Intensity = 0.7f;
    neonClusterEast.LightColor = Color(1.0f, 0.72f, 0.16f, 1.0f);
    lighting.PointLights.push_back(neonClusterEast);

    SpotLight3D stageSpot{};
    stageSpot.Position = Vector3(0.0f, 58.0f, -42.0f);
    stageSpot.Direction = Vector3(0.0f, -0.82f, 0.58f);
    stageSpot.Range = 135.0f;
    stageSpot.InnerConeAngleRadians = 0.38f;
    stageSpot.OuterConeAngleRadians = 0.72f;
    stageSpot.Intensity = 0.42f;
    stageSpot.LightColor = Color(1.0f, 0.91f, 0.78f, 1.0f);
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
