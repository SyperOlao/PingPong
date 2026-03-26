#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "Core/Gameplay/CollisionDebugDraw.h"

#include "Core/Gameplay/Entity.h"
#include "Core/Gameplay/GameplayComponents.h"
#include "Core/Gameplay/Scene.h"
#include "Core/Gameplay/SceneCollisionTypes.h"
#include "Core/Graphics/Debug/DebugDrawQueue.h"
#include "Core/Physics/Collision3D/AxisAlignedBox3D.h"

#include <algorithm>
#include <array>
#include <limits>

namespace
{
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
}

void CollisionDebugDraw::DrawBoxColliderWorldBounds(
    Scene &scene,
    DebugDrawQueue &queue,
    const DirectX::SimpleMath::Color &color
)
{
    scene.ForEachEntityWithTransformAndBoxCollider([&](Entity &entity) {
        TransformComponent *const transform = entity.TryGetTransformComponent();
        BoxColliderComponent *const box = entity.TryGetBoxColliderComponent();
        if (transform == nullptr || box == nullptr)
        {
            return;
        }

        if (box->HalfExtents.x <= 0.0f || box->HalfExtents.y <= 0.0f || box->HalfExtents.z <= 0.0f)
        {
            return;
        }

        const AxisAlignedBox3D worldBounds = ComputeWorldAxisAlignedBoundsForBox(
            transform->WorldMatrix,
            box->LocalCenter,
            box->HalfExtents
        );
        queue.AddAxisAlignedBox(worldBounds, color);
    });
}

void CollisionDebugDraw::DrawContactNormalsFromLastFrame(
    Scene &scene,
    DebugDrawQueue &queue,
    const DirectX::SimpleMath::Color &color,
    const float normalLength
)
{
    for (const CollisionPair &pair : scene.GetCollisionFrameResults())
    {
        if (!pair.Contact.HasOverlap)
        {
            continue;
        }

        if (!pair.Contact.HasContactPoint)
        {
            continue;
        }

        const DirectX::SimpleMath::Vector3 start = pair.Contact.ContactPoint;
        const DirectX::SimpleMath::Vector3 end = start + pair.Contact.Normal * normalLength;
        queue.AddLineSegment(start, end, color);
    }
}

void CollisionDebugDraw::DrawSphereColliderWorldBounds(
    Scene &scene,
    DebugDrawQueue &queue,
    const DirectX::SimpleMath::Color &color
)
{
    scene.ForEachEntityWithTransformAndSphereCollider([&](Entity &entity) {
        TransformComponent *const transform = entity.TryGetTransformComponent();
        SphereColliderComponent *const sphere = entity.TryGetSphereColliderComponent();
        if (transform == nullptr || sphere == nullptr)
        {
            return;
        }

        if (sphere->Radius <= 0.0f)
        {
            return;
        }

        const DirectX::SimpleMath::Vector3 worldCenter = DirectX::SimpleMath::Vector3::Transform(
            sphere->LocalCenter,
            transform->WorldMatrix
        );
        queue.AddWireSphere(worldCenter, sphere->Radius, color, 16);
    });
}

void CollisionDebugDraw::DrawCollisionWorldBounds(
    Scene &scene,
    DebugDrawQueue &queue,
    const DirectX::SimpleMath::Color &color
)
{
    queue.AddAxisAlignedBox(scene.GetCollisionWorldBounds(), color);
}
