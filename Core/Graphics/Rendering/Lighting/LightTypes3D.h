#ifndef PINGPONG_LIGHTTYPES3D_H
#define PINGPONG_LIGHTTYPES3D_H

#include <SimpleMath.h>

struct DirectionalLight3D final
{
    DirectX::SimpleMath::Vector3 Direction{};
    DirectX::SimpleMath::Color LightColor{1.0f, 1.0f, 1.0f, 1.0f};
    float Intensity{1.0f};
};

struct PointLight3D final
{
    DirectX::SimpleMath::Vector3 Position{};
    float Range{10.0f};
    DirectX::SimpleMath::Color LightColor{1.0f, 1.0f, 1.0f, 1.0f};
    float Intensity{1.0f};
};

struct SpotLight3D final
{
    DirectX::SimpleMath::Vector3 Position{};
    float Range{20.0f};
    DirectX::SimpleMath::Vector3 Direction{};
    float InnerConeAngleRadians{0.4f};
    float OuterConeAngleRadians{0.55f};
    DirectX::SimpleMath::Color LightColor{1.0f, 1.0f, 1.0f, 1.0f};
    float Intensity{1.0f};
};

#endif
