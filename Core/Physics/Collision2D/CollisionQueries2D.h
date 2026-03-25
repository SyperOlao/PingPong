#ifndef PINGPONG_COLLISIONQUERIES2D_H
#define PINGPONG_COLLISIONQUERIES2D_H

#include <SimpleMath.h>

#include "Core/Physics/Collision2D/AABB2D.h"
#include "Core/Physics/Collision2D/Circle2D.h"
#include "Core/Physics/Collision2D/CollisionContact2D.h"
#include "Core/Physics/Collision2D/HorizontalBoundsExit.h"

class CollisionQueries2D final
{
public:
    [[nodiscard]] static bool OverlapTestAabbAabb(const AABB2D &firstBox, const AABB2D &secondBox) noexcept;

    [[nodiscard]] static CollisionContact2D FindContactAabbAabb(const AABB2D &firstBox, const AABB2D &secondBox) noexcept;

    static void ReflectVelocityAgainstNormal(
        DirectX::SimpleMath::Vector2 &velocity,
        const DirectX::SimpleMath::Vector2 &normal
    ) noexcept;

    [[nodiscard]] static HorizontalBoundsExit ClassifyHorizontalBoundsExit(
        const AABB2D &axisAlignedBox,
        float leftBoundary,
        float rightBoundary
    ) noexcept;

    [[nodiscard]] static bool OverlapTestPointAabb(
        const DirectX::SimpleMath::Vector2 &point,
        const AABB2D &axisAlignedBox
    ) noexcept;

    [[nodiscard]] static bool OverlapTestCircleCircle(const Circle2D &firstCircle, const Circle2D &secondCircle) noexcept;

    [[nodiscard]] static CollisionContact2D FindContactCircleCircle(
        const Circle2D &firstCircle,
        const Circle2D &secondCircle
    ) noexcept;

    [[nodiscard]] static bool OverlapTestCircleAabb(const Circle2D &circle, const AABB2D &axisAlignedBox) noexcept;

    [[nodiscard]] static CollisionContact2D FindContactCircleAabb(
        const Circle2D &circle,
        const AABB2D &axisAlignedBox
    ) noexcept;

private:
    CollisionQueries2D() = default;
};

#endif
