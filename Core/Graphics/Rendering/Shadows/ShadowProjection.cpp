#include "Core/Graphics/Rendering/Shadows/ShadowProjection.h"

#include <algorithm>
#include <cmath>

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

Matrix ShadowProjection::BuildDirectionalLightView(
    const Vector3 &LightDirection,
    const Vector3 &FocusPoint,
    const Vector3 &WorldUp,
    const float EyeDistanceFromFocus
)
{
    Vector3 NormalizedLightDirection = LightDirection;
    NormalizedLightDirection.Normalize();

    const float FocusDistance = (std::max)(EyeDistanceFromFocus, 1.0e-4f);
    const Vector3 EyePosition = FocusPoint - NormalizedLightDirection * FocusDistance;

    return Matrix::CreateLookAt(EyePosition, FocusPoint, WorldUp);
}

Matrix ShadowProjection::BuildOrthographicProjectionForShadows(
    const float OrthographicWidth,
    const float OrthographicHeight,
    const float NearPlaneDistance,
    const float FarPlaneDistance
)
{
    return Matrix::CreateOrthographic(
        OrthographicWidth,
        OrthographicHeight,
        NearPlaneDistance,
        FarPlaneDistance
    );
}

Matrix ShadowProjection::BuildDirectionalLightViewProjection(
    const Vector3 &LightDirection,
    const Vector3 &FocusPoint,
    const Vector3 &WorldUp,
    const float EyeDistanceFromFocus,
    const float OrthographicWidth,
    const float OrthographicHeight,
    const float NearPlaneDistance,
    const float FarPlaneDistance
)
{
    const Matrix View = BuildDirectionalLightView(LightDirection, FocusPoint, WorldUp, EyeDistanceFromFocus);
    const Matrix Projection = BuildOrthographicProjectionForShadows(
        OrthographicWidth,
        OrthographicHeight,
        NearPlaneDistance,
        FarPlaneDistance
    );

    return View * Projection;
}
