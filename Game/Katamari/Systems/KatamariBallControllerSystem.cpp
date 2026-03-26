#include "Game/Katamari/Systems/KatamariBallControllerSystem.h"

#include "Core/App/AppContext.h"
#include "Core/Gameplay/GameplayComponents.h"
#include "Core/Gameplay/Scene.h"
#include "Core/Graphics/FollowCamera.h"
#include "Core/Input/InputSystem.h"
#include "Core/Input/Keyboard.h"
#include "Core/Math/SpatialMath.h"
#include "Game/Katamari/Data/KatamariWorldContext.h"

using DirectX::SimpleMath::Vector3;

KatamariBallControllerSystem::KatamariBallControllerSystem(KatamariWorldContext *const gameplayWorld) noexcept
    : GameplayWorld(gameplayWorld)
{
}

void KatamariBallControllerSystem::Initialize(Scene &, AppContext &)
{
}

void KatamariBallControllerSystem::Update(Scene &scene, AppContext &context, const float deltaTime)
{
    if (GameplayWorld == nullptr || GameplayWorld->Config == nullptr)
    {
        return;
    }

    const EntityId ballId = GameplayWorld->BallEntityId;
    VelocityComponent *const velocity = scene.TryGetVelocityComponent(ballId);
    TransformComponent *const transform = scene.TryGetTransformComponent(ballId);
    if (velocity == nullptr || transform == nullptr)
    {
        return;
    }

    KatamariGameConfig const &config = *GameplayWorld->Config;
    FollowCamera *const followCamera = GameplayWorld->FollowCameraForMovement;
    Vector3 planarForward(0.0f, 0.0f, 1.0f);
    if (followCamera != nullptr)
    {
        const Vector3 eyePosition = followCamera->GetPosition();
        Vector3 towardBall = transform->Local.Position - eyePosition;
        towardBall.y = 0.0f;
        planarForward = SpatialMath::SafeNormalizeVector3(towardBall, Vector3(0.0f, 0.0f, 1.0f));
    }

    const Vector3 planarRight = SpatialMath::SafeNormalizeVector3(
        planarForward.Cross(Vector3::UnitY),
        Vector3(1.0f, 0.0f, 0.0f)
    );

    Keyboard const &keyboard = context.Input.System->GetKeyboard();
    Vector3 wishDirection(0.0f, 0.0f, 0.0f);
    if (keyboard.IsVirtualKeyDown('W'))
    {
        wishDirection += planarForward;
    }
    if (keyboard.IsVirtualKeyDown('S'))
    {
        wishDirection -= planarForward;
    }
    if (keyboard.IsVirtualKeyDown('D'))
    {
        wishDirection += planarRight;
    }
    if (keyboard.IsVirtualKeyDown('A'))
    {
        wishDirection -= planarRight;
    }

    if (wishDirection.LengthSquared() > 1.0e-8f)
    {
        wishDirection.Normalize();
        velocity->LinearVelocity += wishDirection * (config.BallMoveAcceleration * deltaTime);
    }

    Vector3 horizontalVelocity = velocity->LinearVelocity;
    horizontalVelocity.y = 0.0f;
    const float speed = horizontalVelocity.Length();
    if (speed > config.BallMaxHorizontalSpeed)
    {
        horizontalVelocity *= config.BallMaxHorizontalSpeed / speed;
        velocity->LinearVelocity.x = horizontalVelocity.x;
        velocity->LinearVelocity.z = horizontalVelocity.z;
    }

    const float dragFactor = 1.0f - (std::min)(config.BallHorizontalDrag * deltaTime, 0.95f);
    velocity->LinearVelocity.x *= dragFactor;
    velocity->LinearVelocity.z *= dragFactor;
    velocity->LinearVelocity.y = 0.0f;
}
