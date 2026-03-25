//
// Created by SyperOlao on 25.03.2026.
//

#include "Core/Physics/PlaneConstraint.h"

#include "Core/Math/SpatialMath.h"

void PlaneConstraint::ClampToGroundPlane(
    DirectX::SimpleMath::Vector3 &position,
    const float y
) noexcept {
    SpatialMath::ClampPositionMinWorldY(position, y);
}
