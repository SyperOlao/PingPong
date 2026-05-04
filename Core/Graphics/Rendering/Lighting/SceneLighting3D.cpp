#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "Core/Graphics/Rendering/Lighting/SceneLighting3D.h"

#include <algorithm>
#include <cmath>

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

DirectX::SimpleMath::Vector3 CameraWorldPositionFromViewMatrix(const Matrix &view)
{
    const Matrix invertedView = view.Invert();
    return invertedView.Translation();
}

static void StoreVector3(const DirectX::SimpleMath::Vector3 &value, DirectX::XMFLOAT4 &destination)
{
    destination.x = value.x;
    destination.y = value.y;
    destination.z = value.z;
    destination.w = 0.0f;
}

static void StoreColorRgbIntensity(
    const DirectX::SimpleMath::Color &lightColor,
    const float intensity,
    DirectX::XMFLOAT4 &destination
)
{
    destination.x = lightColor.x * intensity;
    destination.y = lightColor.y * intensity;
    destination.z = lightColor.z * intensity;
    destination.w = intensity;
}

void FillLightsGpuConstants(
    const SceneLightingDescriptor3D &lighting,
    LightsGpuConstants &lightsGpuConstants
)
{
    lightsGpuConstants.TotalLightCount = 0u;
    lightsGpuConstants.Padding0 = 0u;
    lightsGpuConstants.Padding1 = 0u;
    lightsGpuConstants.Padding2 = 0u;

    for (LightGpu &entry : lightsGpuConstants.Lights)
    {
        entry = LightGpu{};
    }

    std::uint32_t writeIndex = 0u;

    for (const DirectionalLight3D &directionalLight : lighting.DirectionalLights)
    {
        if (writeIndex >= kMaximumSceneLights)
        {
            break;
        }

        if (!directionalLight.Enabled)
        {
            continue;
        }

        Vector3 propagation = directionalLight.Direction;
        propagation.Normalize();

        LightGpu &lightGpu = lightsGpuConstants.Lights[writeIndex];
        lightGpu.Position = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
        StoreVector3(propagation, lightGpu.Direction);
        StoreColorRgbIntensity(directionalLight.LightColor, directionalLight.Intensity, lightGpu.Color);

        lightGpu.Parameters.x = static_cast<float>(LightKindGpu::Directional);
        lightGpu.Parameters.y = 0.0f;
        lightGpu.Parameters.z = 0.0f;
        lightGpu.Parameters.w = 0.0f;

        lightGpu.Extension.x = -1.0f;
        lightGpu.Extension.y = 0.0f;
        lightGpu.Extension.z = 0.0f;
        lightGpu.Extension.w = 0.0f;

        ++writeIndex;
    }

    for (const PointLight3D &pointLight : lighting.PointLights)
    {
        if (writeIndex >= kMaximumSceneLights)
        {
            break;
        }

        if (!pointLight.Enabled)
        {
            continue;
        }

        LightGpu &lightGpu = lightsGpuConstants.Lights[writeIndex];
        StoreVector3(pointLight.Position, lightGpu.Position);
        StoreVector3(Vector3(0.0f, 0.0f, 0.0f), lightGpu.Direction);
        StoreColorRgbIntensity(pointLight.LightColor, pointLight.Intensity, lightGpu.Color);

        lightGpu.Parameters.x = static_cast<float>(LightKindGpu::Point);
        lightGpu.Parameters.y = std::max(pointLight.Range, 0.0001f);
        lightGpu.Parameters.z = 0.0f;
        lightGpu.Parameters.w = 0.0f;

        lightGpu.Extension.x = -1.0f;
        lightGpu.Extension.y = 0.0f;
        lightGpu.Extension.z = 0.0f;
        lightGpu.Extension.w = 0.0f;

        ++writeIndex;
    }

    for (const SpotLight3D &spotLight : lighting.SpotLights)
    {
        if (writeIndex >= kMaximumSceneLights)
        {
            break;
        }

        if (!spotLight.Enabled)
        {
            continue;
        }

        Vector3 axis = spotLight.Direction;
        axis.Normalize();

        LightGpu &lightGpu = lightsGpuConstants.Lights[writeIndex];
        StoreVector3(spotLight.Position, lightGpu.Position);
        StoreVector3(axis, lightGpu.Direction);
        StoreColorRgbIntensity(spotLight.LightColor, spotLight.Intensity, lightGpu.Color);

        lightGpu.Parameters.x = static_cast<float>(LightKindGpu::Spot);
        lightGpu.Parameters.y = std::max(spotLight.Range, 0.0001f);
        lightGpu.Parameters.z = std::cos(spotLight.InnerConeAngleRadians);
        lightGpu.Parameters.w = std::cos(spotLight.OuterConeAngleRadians);

        lightGpu.Extension.x = -1.0f;
        lightGpu.Extension.y = 0.0f;
        lightGpu.Extension.z = 0.0f;
        lightGpu.Extension.w = 0.0f;

        ++writeIndex;
    }

    lightsGpuConstants.TotalLightCount = writeIndex;
}
