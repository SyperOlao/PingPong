#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "Core/Physics/Collision3D/CollisionQueries3D.h"

#include <algorithm>
#include <cmath>

#include "Core/Math/SpatialMath.h"

#include <array>
#include <cstddef>

bool CollisionQueries3D::OverlapTestSphereSphere(
    const BoundingSphere3D &firstSphere,
    const BoundingSphere3D &secondSphere
) noexcept
{
    const float distanceSquared = (firstSphere.Center - secondSphere.Center).LengthSquared();
    const float radiusSum = firstSphere.Radius + secondSphere.Radius;
    return distanceSquared <= radiusSum * radiusSum;
}

CollisionContact3D CollisionQueries3D::FindContactSphereSphere(
    const BoundingSphere3D &firstSphere,
    const BoundingSphere3D &secondSphere
) noexcept
{
    CollisionContact3D contact{};
    const DirectX::SimpleMath::Vector3 delta = secondSphere.Center - firstSphere.Center;
    const float distance = delta.Length();
    const float radiusSum = firstSphere.Radius + secondSphere.Radius;

    if (distance > radiusSum)
    {
        return contact;
    }

    contact.HasOverlap = true;

    constexpr float kMinimumSeparation = 1.0e-6f;
    if (distance <= kMinimumSeparation)
    {
        contact.Normal = DirectX::SimpleMath::Vector3::UnitY;
        contact.PenetrationDepth = radiusSum;
        return contact;
    }

    contact.Normal = SpatialMath::SafeNormalizeVector3(delta, DirectX::SimpleMath::Vector3::UnitY);
    contact.PenetrationDepth = radiusSum - distance;
    return contact;
}

bool CollisionQueries3D::OverlapTestSphereAxisAlignedBox(
    const BoundingSphere3D &sphere,
    const AxisAlignedBox3D &axisAlignedBox
) noexcept
{
    const DirectX::SimpleMath::Vector3 closestPoint = axisAlignedBox.ClosestPointTo(sphere.Center);
    const DirectX::SimpleMath::Vector3 delta = closestPoint - sphere.Center;
    return delta.LengthSquared() <= sphere.Radius * sphere.Radius;
}

CollisionContact3D CollisionQueries3D::FindContactSphereAxisAlignedBox(
    const BoundingSphere3D &sphere,
    const AxisAlignedBox3D &axisAlignedBox
) noexcept
{
    CollisionContact3D contact{};
    const DirectX::SimpleMath::Vector3 closestPoint = axisAlignedBox.ClosestPointTo(sphere.Center);
    DirectX::SimpleMath::Vector3 delta = sphere.Center - closestPoint;
    const float distanceSquared = delta.LengthSquared();
    constexpr float kMinimumSeparation = 1.0e-6f;

    if (distanceSquared > kMinimumSeparation * kMinimumSeparation)
    {
        const float distance = std::sqrt(distanceSquared);
        if (distance > sphere.Radius)
        {
            return contact;
        }

        contact.HasOverlap = true;
        contact.Normal = SpatialMath::SafeNormalizeVector3(delta, DirectX::SimpleMath::Vector3::UnitY);
        contact.PenetrationDepth = sphere.Radius - distance;
        contact.HasContactPoint = true;
        contact.ContactPoint = sphere.Center - contact.Normal * sphere.Radius;
        return contact;
    }

    const float distanceToMinX = sphere.Center.x - axisAlignedBox.Min.x;
    const float distanceToMaxX = axisAlignedBox.Max.x - sphere.Center.x;
    const float distanceToMinY = sphere.Center.y - axisAlignedBox.Min.y;
    const float distanceToMaxY = axisAlignedBox.Max.y - sphere.Center.y;
    const float distanceToMinZ = sphere.Center.z - axisAlignedBox.Min.z;
    const float distanceToMaxZ = axisAlignedBox.Max.z - sphere.Center.z;

    float minimumFaceDistance = distanceToMinX;
    DirectX::SimpleMath::Vector3 normal = DirectX::SimpleMath::Vector3(-1.0f, 0.0f, 0.0f);

    if (distanceToMaxX < minimumFaceDistance)
    {
        minimumFaceDistance = distanceToMaxX;
        normal = DirectX::SimpleMath::Vector3(1.0f, 0.0f, 0.0f);
    }
    if (distanceToMinY < minimumFaceDistance)
    {
        minimumFaceDistance = distanceToMinY;
        normal = DirectX::SimpleMath::Vector3(0.0f, -1.0f, 0.0f);
    }
    if (distanceToMaxY < minimumFaceDistance)
    {
        minimumFaceDistance = distanceToMaxY;
        normal = DirectX::SimpleMath::Vector3(0.0f, 1.0f, 0.0f);
    }
    if (distanceToMinZ < minimumFaceDistance)
    {
        minimumFaceDistance = distanceToMinZ;
        normal = DirectX::SimpleMath::Vector3(0.0f, 0.0f, -1.0f);
    }
    if (distanceToMaxZ < minimumFaceDistance)
    {
        minimumFaceDistance = distanceToMaxZ;
        normal = DirectX::SimpleMath::Vector3(0.0f, 0.0f, 1.0f);
    }

    contact.HasOverlap = true;

    contact.Normal = normal;
    contact.PenetrationDepth = sphere.Radius + (std::max)(minimumFaceDistance, 0.0f);
    contact.HasContactPoint = true;
    contact.ContactPoint = sphere.Center - contact.Normal * sphere.Radius;
    return contact;
}

CollisionContact3D CollisionQueries3D::FindContactSphereOrientedBox(
    const BoundingSphere3D &worldSphere,
    const DirectX::SimpleMath::Vector3 &localBoxCenterInEntitySpace,
    const DirectX::SimpleMath::Vector3 &halfExtents,
    const DirectX::SimpleMath::Matrix &entityWorldMatrix
) noexcept
{
    CollisionContact3D contact{};

    const DirectX::SimpleMath::Vector3 boxCenterWorld = DirectX::SimpleMath::Vector3::Transform(
        localBoxCenterInEntitySpace,
        entityWorldMatrix
    );

    std::array<DirectX::SimpleMath::Vector3, 3> axes =
    {
        DirectX::SimpleMath::Vector3(entityWorldMatrix._11, entityWorldMatrix._12, entityWorldMatrix._13),
        DirectX::SimpleMath::Vector3(entityWorldMatrix._21, entityWorldMatrix._22, entityWorldMatrix._23),
        DirectX::SimpleMath::Vector3(entityWorldMatrix._31, entityWorldMatrix._32, entityWorldMatrix._33)
    };
    const std::array<float, 3> localHalfExtents = {halfExtents.x, halfExtents.y, halfExtents.z};
    std::array<float, 3> worldHalfExtents{};

    for (std::size_t axisIndex = 0u; axisIndex < axes.size(); ++axisIndex)
    {
        const float axisLength = axes[axisIndex].Length();
        if (axisLength <= 1.0e-8f)
        {
            axes[axisIndex] = axisIndex == 0u
                ? DirectX::SimpleMath::Vector3::UnitX
                : axisIndex == 1u
                    ? DirectX::SimpleMath::Vector3::UnitY
                    : DirectX::SimpleMath::Vector3::UnitZ;
            worldHalfExtents[axisIndex] = 0.0f;
            continue;
        }

        axes[axisIndex] /= axisLength;
        worldHalfExtents[axisIndex] = localHalfExtents[axisIndex] * axisLength;
    }

    const DirectX::SimpleMath::Vector3 sphereCenterFromBox = worldSphere.Center - boxCenterWorld;
    DirectX::SimpleMath::Vector3 closestPoint = boxCenterWorld;
    std::array<float, 3> projections{};
    bool sphereCenterInsideBox = true;

    for (std::size_t axisIndex = 0u; axisIndex < axes.size(); ++axisIndex)
    {
        projections[axisIndex] = sphereCenterFromBox.Dot(axes[axisIndex]);
        const float clampedProjection = std::clamp(
            projections[axisIndex],
            -worldHalfExtents[axisIndex],
            worldHalfExtents[axisIndex]
        );
        closestPoint += axes[axisIndex] * clampedProjection;
        if (std::abs(projections[axisIndex]) > worldHalfExtents[axisIndex])
        {
            sphereCenterInsideBox = false;
        }
    }

    DirectX::SimpleMath::Vector3 closestToSphere = worldSphere.Center - closestPoint;
    const float distanceSquared = closestToSphere.LengthSquared();
    constexpr float kMinimumSeparation = 1.0e-6f;

    if (distanceSquared > kMinimumSeparation * kMinimumSeparation)
    {
        const float distance = std::sqrt(distanceSquared);
        if (distance > worldSphere.Radius)
        {
            return contact;
        }

        contact.HasOverlap = true;
        contact.Normal = SpatialMath::SafeNormalizeVector3(closestToSphere, DirectX::SimpleMath::Vector3::UnitY);
        contact.PenetrationDepth = worldSphere.Radius - distance;
        contact.HasContactPoint = true;
        contact.ContactPoint = worldSphere.Center - contact.Normal * worldSphere.Radius;
        return contact;
    }

    if (!sphereCenterInsideBox)
    {
        return contact;
    }

    std::size_t nearestFaceAxisIndex = 0u;
    float nearestFaceDistance = worldHalfExtents[0] - std::abs(projections[0]);
    for (std::size_t axisIndex = 1u; axisIndex < axes.size(); ++axisIndex)
    {
        const float faceDistance = worldHalfExtents[axisIndex] - std::abs(projections[axisIndex]);
        if (faceDistance < nearestFaceDistance)
        {
            nearestFaceDistance = faceDistance;
            nearestFaceAxisIndex = axisIndex;
        }
    }

    const float normalSign = projections[nearestFaceAxisIndex] >= 0.0f ? 1.0f : -1.0f;
    contact.HasOverlap = true;
    contact.Normal = axes[nearestFaceAxisIndex] * normalSign;
    contact.PenetrationDepth = worldSphere.Radius + (std::max)(nearestFaceDistance, 0.0f);
    contact.HasContactPoint = true;
    contact.ContactPoint = worldSphere.Center - contact.Normal * worldSphere.Radius;
    return contact;
}

bool CollisionQueries3D::OverlapTestPointAxisAlignedBox(
    const DirectX::SimpleMath::Vector3 &point,
    const AxisAlignedBox3D &axisAlignedBox
) noexcept
{
    return axisAlignedBox.Contains(point);
}

bool CollisionQueries3D::OverlapTestPointSphere(
    const DirectX::SimpleMath::Vector3 &point,
    const BoundingSphere3D &sphere
) noexcept
{
    return (point - sphere.Center).LengthSquared() <= sphere.Radius * sphere.Radius;
}
