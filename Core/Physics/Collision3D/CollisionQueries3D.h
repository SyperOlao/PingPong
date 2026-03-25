#ifndef PINGPONG_COLLISIONQUERIES3D_H
#define PINGPONG_COLLISIONQUERIES3D_H

#include <SimpleMath.h>

#include "Core/Physics/Collision3D/AxisAlignedBox3D.h"
#include "Core/Physics/Collision3D/BoundingSphere3D.h"
#include "Core/Physics/Collision3D/CollisionContact3D.h"

class CollisionQueries3D final
{
public:
    [[nodiscard]] static bool OverlapTestSphereSphere(
        const BoundingSphere3D &firstSphere,
        const BoundingSphere3D &secondSphere
    ) noexcept;

    [[nodiscard]] static CollisionContact3D FindContactSphereSphere(
        const BoundingSphere3D &firstSphere,
        const BoundingSphere3D &secondSphere
    ) noexcept;

    [[nodiscard]] static bool OverlapTestSphereAxisAlignedBox(
        const BoundingSphere3D &sphere,
        const AxisAlignedBox3D &axisAlignedBox
    ) noexcept;

    [[nodiscard]] static CollisionContact3D FindContactSphereAxisAlignedBox(
        const BoundingSphere3D &sphere,
        const AxisAlignedBox3D &axisAlignedBox
    ) noexcept;

    [[nodiscard]] static bool OverlapTestPointAxisAlignedBox(
        const DirectX::SimpleMath::Vector3 &point,
        const AxisAlignedBox3D &axisAlignedBox
    ) noexcept;

    [[nodiscard]] static bool OverlapTestPointSphere(
        const DirectX::SimpleMath::Vector3 &point,
        const BoundingSphere3D &sphere
    ) noexcept;

private:
    CollisionQueries3D() = default;
};

#endif
