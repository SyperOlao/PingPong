#ifndef PINGPONG_SCENELIGHTING3D_H
#define PINGPONG_SCENELIGHTING3D_H

#include "Core/Graphics/Rendering/Lighting/LightTypes3D.h"
#include "Core/Graphics/Rendering/Lighting/ShaderConstants3D.h"

#include <SimpleMath.h>

#include <vector>

struct SceneLightingDescriptor3D final
{
    std::vector<DirectionalLight3D> DirectionalLights{};
    std::vector<PointLight3D> PointLights{};
    std::vector<SpotLight3D> SpotLights{};
};

[[nodiscard]] DirectX::SimpleMath::Vector3 CameraWorldPositionFromViewMatrix(
    const DirectX::SimpleMath::Matrix &view
);

void FillLightsGpuConstants(
    const SceneLightingDescriptor3D &lighting,
    LightsGpuConstants &lightsGpuConstants
);

#endif
