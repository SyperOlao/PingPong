//
// Created by SyperOlao on 25.03.2026.
//

#include "Core/Math/Transform3D.h"

#include "Core/Math/SpatialMath.h"

DirectX::SimpleMath::Matrix Transform3D::GetWorldMatrix() const noexcept {
    return SpatialMath::ComposeWorldMatrixTrs(Scale, RotationEulerRad, Position);
}

DirectX::SimpleMath::Matrix Transform3D::GetRotationMatrix() const noexcept {
    return SpatialMath::RotationMatrixFromEulerXyzRadians(RotationEulerRad);
}

DirectX::SimpleMath::Vector3 Transform3D::GetForward() const noexcept {
    return SpatialMath::GetForwardFromRotation(GetRotationMatrix());
}

DirectX::SimpleMath::Vector3 Transform3D::GetRight() const noexcept {
    return SpatialMath::GetRightFromRotation(GetRotationMatrix());
}

DirectX::SimpleMath::Vector3 Transform3D::GetUp() const noexcept {
    return SpatialMath::GetUpFromRotation(GetRotationMatrix());
}
