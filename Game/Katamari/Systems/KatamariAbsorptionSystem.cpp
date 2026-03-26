#include "Game/Katamari/Systems/KatamariAbsorptionSystem.h"

#include "Core/App/AppContext.h"
#include "Core/Gameplay/Entity.h"
#include "Core/Gameplay/GameplayComponents.h"
#include "Core/Gameplay/Scene.h"
#include "Core/Gameplay/SceneCollisionTypes.h"
#include "Game/Katamari/Data/KatamariWorldContext.h"

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

        if (GameplayWorld->BallRadius < pickupRecord.MinimumBallRadiusToAbsorb)
        {
            continue;
        }

        if (GameplayWorld->BallRadius < pickupRecord.CollisionSphereRadius * config.AbsorbMinimumBallToPickupRadiusRatio)
        {
            continue;
        }

        TransformComponent *const pickupTransform = scene.TryGetTransformComponent(pickupEntityId);
        if (pickupTransform == nullptr)
        {
            continue;
        }

        Entity pickup = scene.GetEntityById(pickupEntityId);
        if (!pickup.IsValid())
        {
            continue;
        }

        const Vector3 ballPosition = ballTransform->Local.Position;
        const Vector3 pickupPosition = pickupTransform->Local.Position;
        const Vector3 offsetWorld = pickupPosition - ballPosition;

        AttachmentComponent attachment{};
        attachment.ParentEntityId = ballId;
        attachment.LocalOffset = offsetWorld;
        pickup.AddAttachmentComponent(attachment);

        pickupTransform->Local.Position = Vector3::Zero;
        pickupTransform->Local.RotationEulerRad = Vector3::Zero;

        scene.RemoveSphereColliderComponent(pickupEntityId);
        scene.RemoveVelocityComponent(pickupEntityId);

        pickupRecord.Absorbed = true;
        GameplayWorld->BallVolume += pickupRecord.AbsorbVolumeContribution;
        GameplayWorld->BallRadius = RadiusFromSphereVolume(GameplayWorld->BallVolume);
        ballCollider->Radius = GameplayWorld->BallRadius;
        ballTransform->Local.Position.y = GameplayWorld->BallRadius;

        ++GameplayWorld->CollectedCount;
    }
}
