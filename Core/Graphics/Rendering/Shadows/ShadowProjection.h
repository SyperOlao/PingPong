#ifndef PINGPONG_SHADOWPROJECTION_H
#define PINGPONG_SHADOWPROJECTION_H

#include <SimpleMath.h>

class ShadowProjection final
{
public:
    [[nodiscard]] static DirectX::SimpleMath::Matrix BuildDirectionalLightView(
        const DirectX::SimpleMath::Vector3 &LightDirection,
        const DirectX::SimpleMath::Vector3 &FocusPoint,
        const DirectX::SimpleMath::Vector3 &WorldUp,
        float EyeDistanceFromFocus
    );

    [[nodiscard]] static DirectX::SimpleMath::Matrix BuildOrthographicProjectionForShadows(
        float OrthographicWidth,
        float OrthographicHeight,
        float NearPlaneDistance,
        float FarPlaneDistance
    );

    [[nodiscard]] static DirectX::SimpleMath::Matrix BuildDirectionalLightViewProjection(
        const DirectX::SimpleMath::Vector3 &LightDirection,
        const DirectX::SimpleMath::Vector3 &FocusPoint,
        const DirectX::SimpleMath::Vector3 &WorldUp,
        float EyeDistanceFromFocus,
        float OrthographicWidth,
        float OrthographicHeight,
        float NearPlaneDistance,
        float FarPlaneDistance
    );
};

#endif
