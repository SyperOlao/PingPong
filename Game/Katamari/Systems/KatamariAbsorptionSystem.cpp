#include "Game/Katamari/Systems/KatamariAbsorptionSystem.h"

#include "Core/App/AppContext.h"
#include "Core/Gameplay/AttachmentTransformHelpers.h"
#include "Core/Gameplay/Entity.h"
#include "Core/Gameplay/GameplayComponents.h"
#include "Core/Gameplay/Scene.h"
#include "Core/Gameplay/SceneCollisionTypes.h"
#include "Game/Katamari/Data/KatamariWorldContext.h"
#include "Game/Katamari/KatamariSphereColliderWorld.h"

#include <cmath>

using DirectX::SimpleMath::Vector3;

namespace
{
[[nodiscard]] float RadiusFromSphereVolume(const float volume) noexcept
{
    constexpr float FourThirdsPi = 4.1887902047863905f;
    if (volume <= 0.0f)
    {
        return 0.0f;
    }

    return std::cbrt(volume / FourThirdsPi);
}
}

KatamariAbsorptionSystem::KatamariAbsorptionSystem(KatamariWorldContext *const gameplayWorld) noexcept
    : GameplayWorld(gameplayWorld)
{
}

void KatamariAbsorptionSystem::Initialize(Scene &, AppContext &)
{
}

void KatamariAbsorptionSystem::Update(Scene &scene, AppContext &, float)
{
    if (GameplayWorld == nullptr || GameplayWorld->Config == nullptr)
    {
        return;
    }

    const EntityId ballId = GameplayWorld->BallEntityId;
    TransformComponent *const ballTransform = scene.TryGetTransformComponent(ballId);
    SphereColliderComponent *const ballCollider = scene.TryGetSphereColliderComponent(ballId);
    if (ballTransform == nullptr || ballCollider == nullptr)
    {
        return;
    }

    KatamariGameConfig const &config = *GameplayWorld->Config;
    std::vector<CollisionPair> const &pairs = scene.GetCollisionFrameResults();

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

        KatamariPickupRecord &pickupRecord = pickupIterator->second;
        if (pickupRecord.Absorbed)
        {
            continue;
        }

        TransformComponent *const pickupTransform = scene.TryGetTransformComponent(pickupEntityId);
        SphereColliderComponent const *const pickupCollider = scene.TryGetSphereColliderComponent(pickupEntityId);
        if (pickupTransform == nullptr || pickupCollider == nullptr)
        {
            continue;
        }

        pickupTransform->WorldMatrix = pickupTransform->Local.GetWorldMatrix();
        ballTransform->WorldMatrix = ballTransform->Local.GetWorldMatrix();

        Vector3 ballWorldCenter;
        float ballWorldRadius;
        KatamariSphereColliderWorld::ComputeWorldCenterAndRadius(*ballTransform, *ballCollider, ballWorldCenter, ballWorldRadius);

        Vector3 pickupWorldCenter;
        float pickupWorldRadius;
        KatamariSphereColliderWorld::ComputeWorldCenterAndRadius(*pickupTransform, *pickupCollider, pickupWorldCenter, pickupWorldRadius);

        const float centerDistance = (pickupWorldCenter - ballWorldCenter).Length();
        const float overlapDistance = ballWorldRadius + pickupWorldRadius;
        if (centerDistance > overlapDistance + 1.0e-3f)
        {
            continue;
        }

        float minimumBallRadiusRequired;
        if (pickupRecord.MinimumBallRadiusToAbsorb > 0.0f)
        {
            minimumBallRadiusRequired = pickupRecord.MinimumBallRadiusToAbsorb;
        }
        else
        {
            minimumBallRadiusRequired = pickupWorldRadius * config.AbsorbMinimumBallToPickupRadiusRatio;
        }

        if (GameplayWorld->BallRadius < minimumBallRadiusRequired)
        {
            continue;
        }

        Entity pickup = scene.GetEntityById(pickupEntityId);
        if (!pickup.IsValid())
        {
            continue;
        }

        if (!AttachmentTransformHelpers::AttachEntityPreserveWorldTransform(scene, pickupEntityId, ballId, Vector3::Zero))
        {
            continue;
        }

        GameplayWorld->BallVolume += pickupRecord.AbsorbVolumeContribution;
        GameplayWorld->BallRadius = RadiusFromSphereVolume(GameplayWorld->BallVolume);
        ballCollider->Radius = GameplayWorld->BallRadius;

        scene.RemoveSphereColliderComponent(pickupEntityId);
        scene.RemoveVelocityComponent(pickupEntityId);

        pickupRecord.Absorbed = true;

        ++GameplayWorld->CollectedCount;
    }
}
