#include "Game/Katamari/Systems/KatamariSphereWorldResolveSystem.h"

#include "Core/App/AppContext.h"
#include "Core/Gameplay/GameplayComponents.h"
#include "Core/Gameplay/Scene.h"
#include "Core/Gameplay/SphereStaticWorldCollisionResolver.h"
#include "Game/Katamari/Data/KatamariWorldContext.h"

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
    SphereStaticWorldCollisionResolver::ApplySphereVersusStaticBoxResponseForEntity(scene, ballId);

    TransformComponent *const transform = scene.TryGetTransformComponent(ballId);
    if (transform == nullptr)
    {
        return;
    }

    if (transform->Local.Position.y < GameplayWorld->BallRadius)
    {
        transform->Local.Position.y = GameplayWorld->BallRadius;
    }
}
