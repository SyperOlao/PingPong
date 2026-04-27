#include "Game/Katamari/Systems/KatamariSphereWorldResolveSystem.h"

#include "Core/App/AppContext.h"
#include "Core/Gameplay/GameplayComponents.h"
#include "Core/Gameplay/Scene.h"
#include "Core/Gameplay/SceneCollisionTypes.h"
#include "Game/Katamari/Data/KatamariWorldContext.h"
#include "Game/Katamari/KatamariSphereColliderWorld.h"

using DirectX::SimpleMath::Vector3;

KatamariSphereWorldResolveSystem::KatamariSphereWorldResolveSystem(KatamariWorldContext *const gameplayWorld) noexcept
    : GameplayWorld(gameplayWorld)
{
}

void KatamariSphereWorldResolveSystem::Initialize(Scene &, AppContext &)
{
}

void KatamariSphereWorldResolveSystem::Update(Scene &scene, AppContext &, float)
{
    if (GameplayWorld == nullptr || GameplayWorld->Config == nullptr)
    {
        return;
    }

    const EntityId ballId = GameplayWorld->BallEntityId;
    TransformComponent *const transform = scene.TryGetTransformComponent(ballId);
    SphereColliderComponent const *const ballCollider = scene.TryGetSphereColliderComponent(ballId);
    if (transform == nullptr || ballCollider == nullptr)
    {
        return;
    }

    VelocityComponent *const velocity = scene.TryGetVelocityComponent(ballId);
    const std::vector<CollisionPair> &pairs = scene.GetCollisionFrameResults();

    for (CollisionPair const &pair : pairs)
    {
        if (pair.Kind != CollisionPairKind::SphereBox)
        {
            continue;
        }

        if (pair.EntityA != ballId)
        {
            continue;
        }

        const BoxColliderComponent *const box = scene.TryGetBoxColliderComponent(pair.EntityB);
        if (box == nullptr || box->IsTrigger || !box->IsStatic)
        {
            continue;
        }

        if (!pair.Contact.HasOverlap)
        {
            continue;
        }

        transform->Local.Position += pair.Contact.Normal * pair.Contact.PenetrationDepth;

        if (velocity != nullptr)
        {
            const float inwardDot = velocity->LinearVelocity.Dot(pair.Contact.Normal);
            if (inwardDot < 0.0f)
            {
                velocity->LinearVelocity -= pair.Contact.Normal * inwardDot;
            }
        }
    }

    transform->WorldMatrix = transform->Local.GetWorldMatrix();

    for (CollisionPair const &pair : pairs)
    {
        if (pair.Kind != CollisionPairKind::SphereSphere)
        {
            continue;
        }

        EntityId pickupEntityId = 0u;
        if (pair.EntityA == ballId)
        {
            pickupEntityId = pair.EntityB;
        }
        else if (pair.EntityB == ballId)
        {
            pickupEntityId = pair.EntityA;
        }
        else
        {
            continue;
        }

        auto pickupIterator = GameplayWorld->Pickups.find(pickupEntityId);
        if (pickupIterator == GameplayWorld->Pickups.end())
        {
            continue;
        }

        KatamariPickupRecord const &pickupRecord = pickupIterator->second;
        if (pickupRecord.Absorbed)
        {
            continue;
        }

        TransformComponent const *const pickupTransform = scene.TryGetTransformComponent(pickupEntityId);
        SphereColliderComponent const *const pickupCollider = scene.TryGetSphereColliderComponent(pickupEntityId);
        if (pickupTransform == nullptr || pickupCollider == nullptr)
        {
            continue;
        }

        transform->WorldMatrix = transform->Local.GetWorldMatrix();

        Vector3 ballWorldCenter;
        float ballWorldRadius;
        KatamariSphereColliderWorld::ComputeWorldCenterAndRadius(*transform, *ballCollider, ballWorldCenter, ballWorldRadius);

        Vector3 pickupWorldCenter;
        float pickupWorldRadius;
        KatamariSphereColliderWorld::ComputeWorldCenterAndRadius(*pickupTransform, *pickupCollider, pickupWorldCenter, pickupWorldRadius);

        Vector3 ballTowardPickup = pickupWorldCenter - ballWorldCenter;
        const float distance = ballTowardPickup.Length();
        const float radiusSum = ballWorldRadius + pickupWorldRadius;
        if (distance >= radiusSum)
        {
            continue;
        }

        const float penetration = radiusSum - distance;
        if (distance > 1.0e-6f)
        {
            ballTowardPickup /= distance;
        }
        else
        {
            ballTowardPickup = Vector3::UnitY;
        }

        transform->Local.Position -= ballTowardPickup * penetration;

        if (velocity != nullptr)
        {
            const float movingIntoPickupDot = velocity->LinearVelocity.Dot(ballTowardPickup);
            if (movingIntoPickupDot > 0.0f)
            {
                velocity->LinearVelocity -= ballTowardPickup * movingIntoPickupDot;
            }
        }
    }

    if (transform->Local.Position.y < GameplayWorld->BallRadius)
    {
        transform->Local.Position.y = GameplayWorld->BallRadius;
    }

    transform->WorldMatrix = transform->Local.GetWorldMatrix();
}
