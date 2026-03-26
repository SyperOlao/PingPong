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

constexpr float PickupAttachPaddingFactor = 0.95f;

void RefreshAbsorbedPickupOffsets(Scene &scene, KatamariWorldContext &world, const float paddingFactor)
{
    for (auto &pickupEntry : world.Pickups)
    {
        const EntityId pickupEntityId = pickupEntry.first;
        KatamariPickupRecord &pickupRecord = pickupEntry.second;
        if (!pickupRecord.Absorbed || !pickupRecord.HasAttachmentDirection)
        {
            continue;
        }

        AttachmentComponent *const attachment = scene.TryGetAttachmentComponent(pickupEntityId);
        if (attachment == nullptr || attachment->ParentEntityId != world.BallEntityId)
        {
            continue;
        }

        const float attachmentDistanceFromBallCenter = world.BallRadius + pickupRecord.CollisionSphereRadius * paddingFactor;
        attachment->LocalOffset = pickupRecord.AttachedDirectionLocal * attachmentDistanceFromBallCenter;
    }
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
        Vector3 directionFromBallToPickup = pickupPosition - ballPosition;
        if (directionFromBallToPickup.LengthSquared() <= 1.0e-8f)
        {
            directionFromBallToPickup = Vector3::UnitX;
        }
        else
        {
            directionFromBallToPickup.Normalize();
        }

        const DirectX::SimpleMath::Matrix inverseBallWorld = ballTransform->WorldMatrix.Invert();
        Vector3 directionLocal = Vector3::TransformNormal(directionFromBallToPickup, inverseBallWorld);
        if (directionLocal.LengthSquared() <= 1.0e-8f)
        {
            directionLocal = Vector3::UnitX;
        }
        else
        {
            directionLocal.Normalize();
        }

        pickupRecord.AttachedDirectionLocal = directionLocal;
        pickupRecord.HasAttachmentDirection = true;

        AttachmentComponent attachment{};
        attachment.ParentEntityId = ballId;
        attachment.LocalOffset = Vector3::Zero;
        pickup.AddAttachmentComponent(attachment);

        pickupTransform->Local.Position = Vector3::Zero;

        scene.RemoveSphereColliderComponent(pickupEntityId);
        scene.RemoveVelocityComponent(pickupEntityId);

        pickupRecord.Absorbed = true;
        GameplayWorld->BallVolume += pickupRecord.AbsorbVolumeContribution;
        GameplayWorld->BallRadius = RadiusFromSphereVolume(GameplayWorld->BallVolume);
        ballCollider->Radius = GameplayWorld->BallRadius;
        RefreshAbsorbedPickupOffsets(scene, *GameplayWorld, PickupAttachPaddingFactor);

        ++GameplayWorld->CollectedCount;
    }
}
