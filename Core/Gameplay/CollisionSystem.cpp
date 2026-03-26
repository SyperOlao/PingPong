#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "Core/Gameplay/CollisionSystem.h"

#include "Core/Gameplay/GameplayComponents.h"
#include "Core/Gameplay/Scene.h"
#include "Core/Gameplay/SceneCollisionTypes.h"
#include "Core/Physics/Collision3D/AxisAlignedBox3D.h"
#include "Core/Physics/Collision3D/BoundingSphere3D.h"
#include "Core/Physics/Collision3D/CollisionQueries3D.h"

#include <algorithm>
#include <array>
#include <limits>

namespace
{
float ComputeMaximumWorldScaleAxis(const DirectX::SimpleMath::Matrix &worldMatrix)
{
    const DirectX::SimpleMath::Vector3 basisX(worldMatrix._11, worldMatrix._12, worldMatrix._13);
    const DirectX::SimpleMath::Vector3 basisY(worldMatrix._21, worldMatrix._22, worldMatrix._23);
    const DirectX::SimpleMath::Vector3 basisZ(worldMatrix._31, worldMatrix._32, worldMatrix._33);

    return (std::max)(basisX.Length(), (std::max)(basisY.Length(), basisZ.Length()));
}

bool LayersOverlapSphereSphere(const SphereColliderComponent &first, const SphereColliderComponent &second)
{
    const std::uint32_t bitFirst = 1u << first.CollisionLayer;
    const std::uint32_t bitSecond = 1u << second.CollisionLayer;
    return (first.CollidesWithMask & bitSecond) != 0u && (second.CollidesWithMask & bitFirst) != 0u;
}

bool LayersOverlapSphereBox(const SphereColliderComponent &sphere, const BoxColliderComponent &box)
{
    const std::uint32_t bitSphere = 1u << sphere.CollisionLayer;
    const std::uint32_t bitBox = 1u << box.CollisionLayer;
    return (sphere.CollidesWithMask & bitBox) != 0u && (box.CollidesWithMask & bitSphere) != 0u;
}

AxisAlignedBox3D ComputeWorldAxisAlignedBoundsForBox(
    const DirectX::SimpleMath::Matrix &entityWorldMatrix,
    const DirectX::SimpleMath::Vector3 &localCenter,
    const DirectX::SimpleMath::Vector3 &halfExtents
)
{
    constexpr std::array<std::array<int, 3>, 8> cornerSigns{{
        {{-1, -1, -1}},
        {{1, -1, -1}},
        {{-1, 1, -1}},
        {{1, 1, -1}},
        {{-1, -1, 1}},
        {{1, -1, 1}},
        {{-1, 1, 1}},
        {{1, 1, 1}},
    }};

    float minimumX = std::numeric_limits<float>::max();
    float minimumY = std::numeric_limits<float>::max();
    float minimumZ = std::numeric_limits<float>::max();
    float maximumX = -std::numeric_limits<float>::max();
    float maximumY = -std::numeric_limits<float>::max();
    float maximumZ = -std::numeric_limits<float>::max();

    for (const std::array<int, 3> &signs : cornerSigns)
    {
        DirectX::SimpleMath::Vector3 localCorner = localCenter;
        localCorner.x += static_cast<float>(signs[0]) * halfExtents.x;
        localCorner.y += static_cast<float>(signs[1]) * halfExtents.y;
        localCorner.z += static_cast<float>(signs[2]) * halfExtents.z;

        const DirectX::SimpleMath::Vector3 worldCorner = DirectX::SimpleMath::Vector3::Transform(
            localCorner,
            entityWorldMatrix
        );

        minimumX = std::min(minimumX, worldCorner.x);
        minimumY = std::min(minimumY, worldCorner.y);
        minimumZ = std::min(minimumZ, worldCorner.z);
        maximumX = std::max(maximumX, worldCorner.x);
        maximumY = std::max(maximumY, worldCorner.y);
        maximumZ = std::max(maximumZ, worldCorner.z);
    }

    AxisAlignedBox3D bounds{};
    bounds.Min = DirectX::SimpleMath::Vector3(minimumX, minimumY, minimumZ);
    bounds.Max = DirectX::SimpleMath::Vector3(maximumX, maximumY, maximumZ);
    return bounds;
}

struct SphereCandidate final
{
    EntityId Id{0u};
    BoundingSphere3D WorldSphere{};
    SphereColliderComponent Collider{};
};

struct BoxCandidate final
{
    EntityId Id{0u};
    DirectX::SimpleMath::Matrix WorldMatrix{DirectX::SimpleMath::Matrix::Identity};
    DirectX::SimpleMath::Vector3 LocalCenter{0.0f, 0.0f, 0.0f};
    DirectX::SimpleMath::Vector3 HalfExtents{0.5f, 0.5f, 0.5f};
    BoxColliderComponent Collider{};
    AxisAlignedBox3D WorldBoundsForBroadphase{};
};

}

void CollisionSystem::Initialize(Scene &, AppContext &)
{
    m_queryScratch.reserve(4096u);
}

void CollisionSystem::Update(Scene &scene, AppContext &, float)
{
    std::vector<SphereCandidate> spheres{};
    spheres.reserve(scene.m_slots.size());

    std::vector<BoxCandidate> boxes{};
    boxes.reserve(scene.m_slots.size());

    for (std::size_t slotIndex = 0; slotIndex < scene.m_slots.size(); ++slotIndex)
    {
        const Scene::EntitySlot &slot = scene.m_slots[slotIndex];
        if (!slot.Active || !slot.Transform.has_value())
        {
            continue;
        }

        if (slot.SphereCollider.has_value())
        {
            const DirectX::SimpleMath::Vector3 worldCenter = DirectX::SimpleMath::Vector3::Transform(
                slot.SphereCollider->LocalCenter,
                slot.Transform->WorldMatrix
            );
            const float worldScaleMaximumAxis = ComputeMaximumWorldScaleAxis(slot.Transform->WorldMatrix);
            const float worldRadius = slot.SphereCollider->Radius * worldScaleMaximumAxis;

            SphereCandidate sphereCandidate{};
            sphereCandidate.Id = slot.Id;
            sphereCandidate.WorldSphere.Center = worldCenter;
            sphereCandidate.WorldSphere.Radius = worldRadius;
            sphereCandidate.Collider = *slot.SphereCollider;
            spheres.push_back(sphereCandidate);
        }

        if (slot.BoxCollider.has_value())
        {
            BoxCandidate boxCandidate{};
            boxCandidate.Id = slot.Id;
            boxCandidate.WorldMatrix = slot.Transform->WorldMatrix;
            boxCandidate.LocalCenter = slot.BoxCollider->LocalCenter;
            boxCandidate.HalfExtents = slot.BoxCollider->HalfExtents;
            boxCandidate.Collider = *slot.BoxCollider;
            boxCandidate.WorldBoundsForBroadphase = ComputeWorldAxisAlignedBoundsForBox(
                slot.Transform->WorldMatrix,
                slot.BoxCollider->LocalCenter,
                slot.BoxCollider->HalfExtents
            );
            boxes.push_back(boxCandidate);
        }
    }

    std::vector<CollisionPair> pairs{};
    pairs.reserve(spheres.size() * 2u + spheres.size() * boxes.size());

    const float cellSize = scene.GetCollisionCellSize();
    const AxisAlignedBox3D worldBounds = scene.GetCollisionWorldBounds();

    if (spheres.size() >= 2u)
    {
        if (cellSize <= 0.0f)
        {
            for (std::size_t indexA = 0; indexA < spheres.size(); ++indexA)
            {
                for (std::size_t indexB = indexA + 1; indexB < spheres.size(); ++indexB)
                {
                    const SphereCandidate &first = spheres[indexA];
                    const SphereCandidate &second = spheres[indexB];
                    if (!LayersOverlapSphereSphere(first.Collider, second.Collider))
                    {
                        continue;
                    }

                    if (!CollisionQueries3D::OverlapTestSphereSphere(first.WorldSphere, second.WorldSphere))
                    {
                        continue;
                    }

                    const CollisionContact3D contact = CollisionQueries3D::FindContactSphereSphere(
                        first.WorldSphere,
                        second.WorldSphere
                    );
                    if (!contact.HasOverlap)
                    {
                        continue;
                    }

                    CollisionPair pair{};
                    pair.Kind = CollisionPairKind::SphereSphere;
                    if (first.Id < second.Id)
                    {
                        pair.EntityA = first.Id;
                        pair.EntityB = second.Id;
                    }
                    else
                    {
                        pair.EntityA = second.Id;
                        pair.EntityB = first.Id;
                    }

                    pair.Contact = contact;
                    pairs.push_back(pair);
                }
            }
        }
        else
        {
            m_spatialGrid.Initialize(cellSize, worldBounds);
            m_spatialGrid.Clear();

            for (std::uint32_t candidateIndex = 0u; candidateIndex < static_cast<std::uint32_t>(spheres.size());
                 ++candidateIndex)
            {
                const SphereCandidate &candidate = spheres[candidateIndex];
                const AxisAlignedBox3D bounds = AxisAlignedBox3D::FromCenterExtents(
                    candidate.WorldSphere.Center,
                    DirectX::SimpleMath::Vector3(
                        candidate.WorldSphere.Radius,
                        candidate.WorldSphere.Radius,
                        candidate.WorldSphere.Radius
                    )
                );
                m_spatialGrid.InsertObject(candidateIndex, bounds);
            }

            for (std::uint32_t candidateIndex = 0u; candidateIndex < static_cast<std::uint32_t>(spheres.size());
                 ++candidateIndex)
            {
                const SphereCandidate &first = spheres[candidateIndex];
                const AxisAlignedBox3D queryBounds = AxisAlignedBox3D::FromCenterExtents(
                    first.WorldSphere.Center,
                    DirectX::SimpleMath::Vector3(
                        first.WorldSphere.Radius,
                        first.WorldSphere.Radius,
                        first.WorldSphere.Radius
                    )
                );

                m_spatialGrid.QueryOverlapping(queryBounds, m_queryScratch);

                for (const std::uint32_t otherIndex : m_queryScratch)
                {
                    if (otherIndex <= candidateIndex)
                    {
                        continue;
                    }

                    const SphereCandidate &second = spheres[otherIndex];
                    if (!LayersOverlapSphereSphere(first.Collider, second.Collider))
                    {
                        continue;
                    }

                    if (!CollisionQueries3D::OverlapTestSphereSphere(first.WorldSphere, second.WorldSphere))
                    {
                        continue;
                    }

                    const CollisionContact3D contact = CollisionQueries3D::FindContactSphereSphere(
                        first.WorldSphere,
                        second.WorldSphere
                    );
                    if (!contact.HasOverlap)
                    {
                        continue;
                    }

                    CollisionPair pair{};
                    pair.Kind = CollisionPairKind::SphereSphere;
                    if (first.Id < second.Id)
                    {
                        pair.EntityA = first.Id;
                        pair.EntityB = second.Id;
                    }
                    else
                    {
                        pair.EntityA = second.Id;
                        pair.EntityB = first.Id;
                    }

                    pair.Contact = contact;
                    pairs.push_back(pair);
                }
            }
        }
    }

    if (!spheres.empty() && !boxes.empty())
    {
        if (cellSize <= 0.0f)
        {
            for (const SphereCandidate &sphereCandidate : spheres)
            {
                for (const BoxCandidate &boxCandidate : boxes)
                {
                    if (!LayersOverlapSphereBox(sphereCandidate.Collider, boxCandidate.Collider))
                    {
                        continue;
                    }

                    const CollisionContact3D contact = CollisionQueries3D::FindContactSphereOrientedBox(
                        sphereCandidate.WorldSphere,
                        boxCandidate.LocalCenter,
                        boxCandidate.HalfExtents,
                        boxCandidate.WorldMatrix
                    );
                    if (!contact.HasOverlap)
                    {
                        continue;
                    }

                    CollisionPair pair{};
                    pair.Kind = CollisionPairKind::SphereBox;
                    pair.EntityA = sphereCandidate.Id;
                    pair.EntityB = boxCandidate.Id;
                    pair.Contact = contact;
                    pairs.push_back(pair);
                }
            }
        }
        else
        {
            m_spatialGrid.Initialize(cellSize, worldBounds);
            m_spatialGrid.Clear();

            for (std::uint32_t boxIndex = 0u; boxIndex < static_cast<std::uint32_t>(boxes.size()); ++boxIndex)
            {
                m_spatialGrid.InsertObject(boxIndex, boxes[boxIndex].WorldBoundsForBroadphase);
            }

            for (const SphereCandidate &sphereCandidate : spheres)
            {
                const AxisAlignedBox3D queryBounds = AxisAlignedBox3D::FromCenterExtents(
                    sphereCandidate.WorldSphere.Center,
                    DirectX::SimpleMath::Vector3(
                        sphereCandidate.WorldSphere.Radius,
                        sphereCandidate.WorldSphere.Radius,
                        sphereCandidate.WorldSphere.Radius
                    )
                );

                m_spatialGrid.QueryOverlapping(queryBounds, m_queryScratch);

                for (const std::uint32_t boxIndex : m_queryScratch)
                {
                    const BoxCandidate &boxCandidate = boxes[boxIndex];
                    if (!LayersOverlapSphereBox(sphereCandidate.Collider, boxCandidate.Collider))
                    {
                        continue;
                    }

                    const CollisionContact3D contact = CollisionQueries3D::FindContactSphereOrientedBox(
                        sphereCandidate.WorldSphere,
                        boxCandidate.LocalCenter,
                        boxCandidate.HalfExtents,
                        boxCandidate.WorldMatrix
                    );
                    if (!contact.HasOverlap)
                    {
                        continue;
                    }

                    CollisionPair pair{};
                    pair.Kind = CollisionPairKind::SphereBox;
                    pair.EntityA = sphereCandidate.Id;
                    pair.EntityB = boxCandidate.Id;
                    pair.Contact = contact;
                    pairs.push_back(pair);
                }
            }
        }
    }

    scene.ReplaceCollisionFrameResults(std::move(pairs));
}
