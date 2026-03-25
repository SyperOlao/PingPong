//
// Created by SyperOlao on 25.03.2026.
//
#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef PINGPONG_SPATIALMATH_H
#define PINGPONG_SPATIALMATH_H

#include <SimpleMath.h>
#include <cmath>
#include <algorithm>

class SpatialMath final {
public:
    static constexpr float EpsilonDirectionSquared = 1.0e-12f;

    [[nodiscard]] static DirectX::SimpleMath::Matrix RotationMatrixFromEulerXyzRadians(
        const DirectX::SimpleMath::Vector3 &rotationEulerRad
    ) noexcept;

    [[nodiscard]] static DirectX::SimpleMath::Matrix ComposeWorldMatrixTrs(
        const DirectX::SimpleMath::Vector3 &scale,
        const DirectX::SimpleMath::Vector3 &rotationEulerRad,
        const DirectX::SimpleMath::Vector3 &translation
    ) noexcept;

    [[nodiscard]] static DirectX::SimpleMath::Vector3 SafeNormalizeVector3(
        const DirectX::SimpleMath::Vector3 &value,
        const DirectX::SimpleMath::Vector3 &fallback
    ) noexcept;

    [[nodiscard]] static DirectX::SimpleMath::Vector3 TransformDirection(
        const DirectX::SimpleMath::Vector3 &localDirection,
        const DirectX::SimpleMath::Matrix &rotationMatrix
    ) noexcept;

    [[nodiscard]] static DirectX::SimpleMath::Vector3 GetRightFromRotation(
        const DirectX::SimpleMath::Matrix &rotationMatrix
    ) noexcept;

    [[nodiscard]] static DirectX::SimpleMath::Vector3 GetUpFromRotation(
        const DirectX::SimpleMath::Matrix &rotationMatrix
    ) noexcept;

    [[nodiscard]] static DirectX::SimpleMath::Vector3 GetForwardFromRotation(
        const DirectX::SimpleMath::Matrix &rotationMatrix
    ) noexcept;

    [[nodiscard]] static DirectX::SimpleMath::Vector3 ForwardFromYawPitchRadians(
        const float yawRadians,
        const float pitchRadians
    ) noexcept;

    [[nodiscard]] static DirectX::SimpleMath::Vector3 RightFromForwardAndWorldUp(
        const DirectX::SimpleMath::Vector3 &forward,
        const DirectX::SimpleMath::Vector3 &worldUp
    ) noexcept;

    [[nodiscard]] static float SignedDistancePointPlane(
        const DirectX::SimpleMath::Vector3 &point,
        const DirectX::SimpleMath::Vector3 &planePoint,
        const DirectX::SimpleMath::Vector3 &planeUnitNormal
    ) noexcept;

    [[nodiscard]] static DirectX::SimpleMath::Vector3 ProjectPointOnPlane(
        const DirectX::SimpleMath::Vector3 &point,
        const DirectX::SimpleMath::Vector3 &planePoint,
        const DirectX::SimpleMath::Vector3 &planeUnitNormal
    ) noexcept;

    static void ClampPositionMinWorldY(
        DirectX::SimpleMath::Vector3 &position,
        const float minimumY
    ) noexcept;

private:
    SpatialMath() = default;
};

inline DirectX::SimpleMath::Matrix SpatialMath::RotationMatrixFromEulerXyzRadians(
    const DirectX::SimpleMath::Vector3 &rotationEulerRad
) noexcept {
    using namespace DirectX::SimpleMath;

    return Matrix::CreateRotationX(rotationEulerRad.x) * Matrix::CreateRotationY(rotationEulerRad.y) *
           Matrix::CreateRotationZ(rotationEulerRad.z);
}

inline DirectX::SimpleMath::Matrix SpatialMath::ComposeWorldMatrixTrs(
    const DirectX::SimpleMath::Vector3 &scale,
    const DirectX::SimpleMath::Vector3 &rotationEulerRad,
    const DirectX::SimpleMath::Vector3 &translation
) noexcept {
    using namespace DirectX::SimpleMath;

    return Matrix::CreateScale(scale) * RotationMatrixFromEulerXyzRadians(rotationEulerRad) *
           Matrix::CreateTranslation(translation);
}

inline DirectX::SimpleMath::Vector3 SpatialMath::SafeNormalizeVector3(
    const DirectX::SimpleMath::Vector3 &value,
    const DirectX::SimpleMath::Vector3 &fallback
) noexcept {
    if (value.LengthSquared() <= EpsilonDirectionSquared) {
        return fallback;
    }

    DirectX::SimpleMath::Vector3 copy = value;
    copy.Normalize();
    return copy;
}

inline DirectX::SimpleMath::Vector3 SpatialMath::TransformDirection(
    const DirectX::SimpleMath::Vector3 &localDirection,
    const DirectX::SimpleMath::Matrix &rotationMatrix
) noexcept {
    return DirectX::SimpleMath::Vector3::TransformNormal(localDirection, rotationMatrix);
}

inline DirectX::SimpleMath::Vector3 SpatialMath::GetRightFromRotation(
    const DirectX::SimpleMath::Matrix &rotationMatrix
) noexcept {
    return SafeNormalizeVector3(
        TransformDirection(DirectX::SimpleMath::Vector3::UnitX, rotationMatrix),
        DirectX::SimpleMath::Vector3::UnitX
    );
}

inline DirectX::SimpleMath::Vector3 SpatialMath::GetUpFromRotation(
    const DirectX::SimpleMath::Matrix &rotationMatrix
) noexcept {
    return SafeNormalizeVector3(
        TransformDirection(DirectX::SimpleMath::Vector3::UnitY, rotationMatrix),
        DirectX::SimpleMath::Vector3::UnitY
    );
}

inline DirectX::SimpleMath::Vector3 SpatialMath::GetForwardFromRotation(
    const DirectX::SimpleMath::Matrix &rotationMatrix
) noexcept {
    return SafeNormalizeVector3(
        TransformDirection(DirectX::SimpleMath::Vector3::UnitZ, rotationMatrix),
        DirectX::SimpleMath::Vector3::UnitZ
    );
}

inline DirectX::SimpleMath::Vector3 SpatialMath::ForwardFromYawPitchRadians(
    const float yawRadians,
    const float pitchRadians
) noexcept {
    const float cosPitch = std::cos(pitchRadians);
    const float sinPitch = std::sin(pitchRadians);
    const float cosYaw = std::cos(yawRadians);
    const float sinYaw = std::sin(yawRadians);

    DirectX::SimpleMath::Vector3 forward{};
    forward.x = sinYaw * cosPitch;
    forward.y = sinPitch;
    forward.z = cosYaw * cosPitch;

    return SafeNormalizeVector3(forward, DirectX::SimpleMath::Vector3::UnitZ);
}

inline DirectX::SimpleMath::Vector3 SpatialMath::RightFromForwardAndWorldUp(
    const DirectX::SimpleMath::Vector3 &forward,
    const DirectX::SimpleMath::Vector3 &worldUp
) noexcept {
    DirectX::SimpleMath::Vector3 right = forward.Cross(worldUp);
    return SafeNormalizeVector3(right, DirectX::SimpleMath::Vector3::UnitX);
}

inline float SpatialMath::SignedDistancePointPlane(
    const DirectX::SimpleMath::Vector3 &point,
    const DirectX::SimpleMath::Vector3 &planePoint,
    const DirectX::SimpleMath::Vector3 &planeUnitNormal
) noexcept {
    return (point - planePoint).Dot(planeUnitNormal);
}

inline DirectX::SimpleMath::Vector3 SpatialMath::ProjectPointOnPlane(
    const DirectX::SimpleMath::Vector3 &point,
    const DirectX::SimpleMath::Vector3 &planePoint,
    const DirectX::SimpleMath::Vector3 &planeUnitNormal
) noexcept {
    return point - planeUnitNormal * SignedDistancePointPlane(point, planePoint, planeUnitNormal);
}

inline void SpatialMath::ClampPositionMinWorldY(
    DirectX::SimpleMath::Vector3& position,
    const float minimumY
) noexcept {
    position.y = position.y < minimumY ? minimumY : position.y;
}

#endif //PINGPONG_SPATIALMATH_H
