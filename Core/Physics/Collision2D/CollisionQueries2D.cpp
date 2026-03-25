#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "Core/Physics/Collision2D/CollisionQueries2D.h"

#include <algorithm>
#include <cmath>

#include "Core/Math/MathHelpers.h"

bool CollisionQueries2D::OverlapTestAabbAabb(const AABB2D &firstBox, const AABB2D &secondBox) noexcept
{
    return firstBox.Intersects(secondBox);
}

CollisionContact2D CollisionQueries2D::FindContactAabbAabb(const AABB2D &firstBox, const AABB2D &secondBox) noexcept
{
    CollisionContact2D contact{};
    if (!OverlapTestAabbAabb(firstBox, secondBox))
    {
        return contact;
    }

    const auto firstCenter = firstBox.GetCenter();
    const auto secondCenter = secondBox.GetCenter();
    const auto firstExtents = firstBox.GetExtents();
    const auto secondExtents = secondBox.GetExtents();

    const float deltaX = secondCenter.x - firstCenter.x;
    const float deltaY = secondCenter.y - firstCenter.y;

    const float overlapX = (firstExtents.x + secondExtents.x) - std::abs(deltaX);
    const float overlapY = (firstExtents.y + secondExtents.y) - std::abs(deltaY);

    contact.HasOverlap = true;

    if (overlapX < overlapY)
    {
        contact.PenetrationDepth = overlapX;
        contact.Normal = DirectX::SimpleMath::Vector2(
            deltaX < 0.0f ? -1.0f : 1.0f,
            0.0f
        );
    }
    else
    {
        contact.PenetrationDepth = overlapY;
        contact.Normal = DirectX::SimpleMath::Vector2(
            0.0f,
            deltaY < 0.0f ? -1.0f : 1.0f
        );
    }

    return contact;
}

void CollisionQueries2D::ReflectVelocityAgainstNormal(
    DirectX::SimpleMath::Vector2 &velocity,
    const DirectX::SimpleMath::Vector2 &normal
) noexcept
{
    velocity = MathHelpers::Reflect(velocity, normal);
}

HorizontalBoundsExit CollisionQueries2D::ClassifyHorizontalBoundsExit(
    const AABB2D &axisAlignedBox,
    const float leftBoundary,
    const float rightBoundary
) noexcept
{
    if (axisAlignedBox.Max.x < leftBoundary)
    {
        return HorizontalBoundsExit::Left;
    }

    if (axisAlignedBox.Min.x > rightBoundary)
    {
        return HorizontalBoundsExit::Right;
    }

    return HorizontalBoundsExit::None;
}

bool CollisionQueries2D::OverlapTestPointAabb(
    const DirectX::SimpleMath::Vector2 &point,
    const AABB2D &axisAlignedBox
) noexcept
{
    return axisAlignedBox.Contains(point);
}

bool CollisionQueries2D::OverlapTestCircleCircle(const Circle2D &firstCircle, const Circle2D &secondCircle) noexcept
{
    const float radiusSum = firstCircle.Radius + secondCircle.Radius;
    const DirectX::SimpleMath::Vector2 delta = secondCircle.Center - firstCircle.Center;
    return delta.LengthSquared() <= radiusSum * radiusSum;
}

CollisionContact2D CollisionQueries2D::FindContactCircleCircle(
    const Circle2D &firstCircle,
    const Circle2D &secondCircle
) noexcept
{
    CollisionContact2D contact{};
    const DirectX::SimpleMath::Vector2 delta = secondCircle.Center - firstCircle.Center;
    const float distanceSquared = delta.LengthSquared();
    const float radiusSum = firstCircle.Radius + secondCircle.Radius;

    if (distanceSquared > radiusSum * radiusSum)
    {
        return contact;
    }

    contact.HasOverlap = true;

    constexpr float kMinimumSeparation = 1.0e-6f;
    const float distance = std::sqrt(distanceSquared);

    if (distance <= kMinimumSeparation)
    {
        contact.Normal = DirectX::SimpleMath::Vector2::UnitX;
        contact.PenetrationDepth = radiusSum;
        return contact;
    }

    contact.Normal = delta / distance;
    contact.PenetrationDepth = radiusSum - distance;
    return contact;
}

bool CollisionQueries2D::OverlapTestCircleAabb(const Circle2D &circle, const AABB2D &axisAlignedBox) noexcept
{
    const float closestX = std::clamp(circle.Center.x, axisAlignedBox.Min.x, axisAlignedBox.Max.x);
    const float closestY = std::clamp(circle.Center.y, axisAlignedBox.Min.y, axisAlignedBox.Max.y);
    const float deltaX = circle.Center.x - closestX;
    const float deltaY = circle.Center.y - closestY;
    return deltaX * deltaX + deltaY * deltaY <= circle.Radius * circle.Radius;
}

CollisionContact2D CollisionQueries2D::FindContactCircleAabb(
    const Circle2D &circle,
    const AABB2D &axisAlignedBox
) noexcept
{
    CollisionContact2D contact{};
    const float closestX = std::clamp(circle.Center.x, axisAlignedBox.Min.x, axisAlignedBox.Max.x);
    const float closestY = std::clamp(circle.Center.y, axisAlignedBox.Min.y, axisAlignedBox.Max.y);
    const float deltaX = circle.Center.x - closestX;
    const float deltaY = circle.Center.y - closestY;
    const float distanceSquared = deltaX * deltaX + deltaY * deltaY;

    if (distanceSquared > circle.Radius * circle.Radius)
    {
        return contact;
    }

    contact.HasOverlap = true;

    constexpr float kMinimumSeparation = 1.0e-6f;

    if (distanceSquared <= kMinimumSeparation * kMinimumSeparation)
    {
        const float centerX = (axisAlignedBox.Min.x + axisAlignedBox.Max.x) * 0.5f;
        const float centerY = (axisAlignedBox.Min.y + axisAlignedBox.Max.y) * 0.5f;
        const float offsetX = circle.Center.x - centerX;
        const float offsetY = circle.Center.y - centerY;
        const float halfWidth = (axisAlignedBox.Max.x - axisAlignedBox.Min.x) * 0.5f;
        const float halfHeight = (axisAlignedBox.Max.y - axisAlignedBox.Min.y) * 0.5f;

        if (halfWidth < halfHeight)
        {
            contact.Normal = DirectX::SimpleMath::Vector2(offsetX < 0.0f ? -1.0f : 1.0f, 0.0f);
            contact.PenetrationDepth = circle.Radius + halfWidth - std::abs(offsetX);
        }
        else
        {
            contact.Normal = DirectX::SimpleMath::Vector2(0.0f, offsetY < 0.0f ? -1.0f : 1.0f);
            contact.PenetrationDepth = circle.Radius + halfHeight - std::abs(offsetY);
        }
        return contact;
    }

    const float distance = std::sqrt(distanceSquared);
    contact.Normal = DirectX::SimpleMath::Vector2(deltaX / distance, deltaY / distance);
    contact.PenetrationDepth = circle.Radius - distance;
    return contact;
}
