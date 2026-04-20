#include "Game/Katamari/Systems/KatamariBallControllerSystem.h"

#include "Core/App/AppContext.h"
#include "Core/Gameplay/GameplayComponents.h"
#include "Core/Gameplay/Scene.h"
#include "Core/Graphics/FollowCamera.h"
#include "Core/Input/InputSystem.h"
#include "Core/Input/Keyboard.h"
#include "Core/Math/SpatialMath.h"
#include "Game/Katamari/Data/KatamariWorldContext.h"

#include <algorithm>

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
        Vector3 cameraMovementDirection = followCamera->GetMovementDirectionXZ();
        cameraMovementDirection.y = 0.0f;
        planarForward = SpatialMath::SafeNormalizeVector3(cameraMovementDirection, Vector3(0.0f, 0.0f, 1.0f));
    }

    const Vector3 planarRight = SpatialMath::SafeNormalizeVector3(
        Vector3::UnitY.Cross(planarForward),
        Vector3(1.0f, 0.0f, 0.0f)
    );

    Keyboard const &keyboard = context.Input.System->GetKeyboard();
    const float groundedHeightThreshold = GameplayWorld->BallRadius + 0.02f;
    const bool IsGrounded = transform->Local.Position.y <= groundedHeightThreshold;
    if (keyboard.WasVirtualKeyPressed(VK_SPACE) && IsGrounded)
    {
        velocity->LinearVelocity.y = config.BallJumpVelocity;
    }

    velocity->LinearVelocity.y -= config.BallGravityAcceleration * deltaTime;

    float forwardInput = 0.0f;
    float rightInput = 0.0f;
    if (keyboard.IsVirtualKeyDown('W'))
    {
        forwardInput += 1.0f;
    }
    if (keyboard.IsVirtualKeyDown('S'))
    {
        forwardInput -= 1.0f;
    }
    if (keyboard.IsVirtualKeyDown('D'))
    {
        rightInput += 1.0f;
    }
    if (keyboard.IsVirtualKeyDown('A'))
    {
        rightInput -= 1.0f;
    }

    Vector3 wishDirection = planarForward * forwardInput + planarRight * rightInput;
    const bool hasMovementInput = wishDirection.LengthSquared() > 1.0e-8f;
    if (hasMovementInput)
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

}
