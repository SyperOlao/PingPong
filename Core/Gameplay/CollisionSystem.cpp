#include "Core/Gameplay/CollisionSystem.h"

#include "Core/App/AppContext.h"
#include "Core/Gameplay/GameplayComponents.h"
#include "Core/Gameplay/Scene.h"
#include "Core/Gameplay/SceneCollisionTypes.h"
#include "Core/Physics/Collision3D/AxisAlignedBox3D.h"
#include "Core/Physics/Collision3D/BoundingSphere3D.h"
#include "Core/Physics/Collision3D/CollisionQueries3D.h"

namespace
{
bool LayersOverlap(const SphereColliderComponent &first, const SphereColliderComponent &second)
{
    const std::uint32_t bitFirst = 1u << first.CollisionLayer;
    const std::uint32_t bitSecond = 1u << second.CollisionLayer;
    return (first.CollidesWithMask & bitSecond) != 0u && (second.CollidesWithMask & bitFirst) != 0u;
}
}

void CollisionSystem::Initialize(Scene &, AppContext &)
{
    m_queryScratch.reserve(4096u);
}

void CollisionSystem::Update(Scene &scene, AppContext &, float)
{
    struct Candidate final
    {
        EntityId Id{0u};
        BoundingSphere3D WorldSphere{};
        SphereColliderComponent Collider{};
    };

    std::vector<Candidate> candidates{};
    candidates.reserve(scene.m_slots.size());

    for (std::size_t slotIndex = 0; slotIndex < scene.m_slots.size(); ++slotIndex)
    {
        const Scene::EntitySlot &slot = scene.m_slots[slotIndex];
        if (!slot.Active)
        {
            continue;
        }

        if (!slot.Transform.has_value() || !slot.SphereCollider.has_value())
        {
            continue;
        }

        const DirectX::SimpleMath::Vector3 worldCenter = DirectX::SimpleMath::Vector3::Transform(
            slot.SphereCollider->LocalCenter,
            slot.Transform->WorldMatrix
        );

        Candidate candidate{};
        candidate.Id = slot.Id;
        candidate.WorldSphere.Center = worldCenter;
        candidate.WorldSphere.Radius = slot.SphereCollider->Radius;
        candidate.Collider = *slot.SphereCollider;
        candidates.push_back(candidate);
    }

    std::vector<CollisionPair> pairs{};
    if (candidates.size() < 2u)
    {
        scene.ReplaceCollisionFrameResults(std::move(pairs));
        return;
    }

    const float cellSize = scene.GetCollisionCellSize();
    if (cellSize <= 0.0f)
    {
        for (std::size_t indexA = 0; indexA < candidates.size(); ++indexA)
        {
            for (std::size_t indexB = indexA + 1; indexB < candidates.size(); ++indexB)
            {
                const Candidate &first = candidates[indexA];
                const Candidate &second = candidates[indexB];
                if (!LayersOverlap(first.Collider, second.Collider))
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

        scene.ReplaceCollisionFrameResults(std::move(pairs));
        return;
    }

    m_spatialGrid.Initialize(cellSize, scene.GetCollisionWorldBounds());
    m_spatialGrid.Clear();

    for (std::uint32_t candidateIndex = 0u; candidateIndex < static_cast<std::uint32_t>(candidates.size()); ++candidateIndex)
    {
        const Candidate &candidate = candidates[candidateIndex];
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

    for (std::uint32_t candidateIndex = 0u; candidateIndex < static_cast<std::uint32_t>(candidates.size()); ++candidateIndex)
    {
        const Candidate &first = candidates[candidateIndex];
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

            const Candidate &second = candidates[otherIndex];
            if (!LayersOverlap(first.Collider, second.Collider))
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

    scene.ReplaceCollisionFrameResults(std::move(pairs));
}
