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

namespace
{
constexpr float GroundedHeightEpsilon = 0.02f;
constexpr float MinimumInputSquaredLength = 1.0e-6f;
constexpr float MaximumDragPerFrame = 0.95f;

Vector3 ResolveCameraPlanarForwardToBall(
    FollowCamera *const followCamera,
    Vector3 const &ballWorldPosition
) noexcept
{
    if (followCamera == nullptr)
    {
        return Vector3(0.0f, 0.0f, 1.0f);
    }

    Vector3 cameraToBallDirection = ballWorldPosition - followCamera->GetPosition();
    cameraToBallDirection.y = 0.0f;
    if (cameraToBallDirection.LengthSquared() > 1.0e-6f)
    {
        cameraToBallDirection.Normalize();
        return cameraToBallDirection;
    }

    Vector3 fallbackForward = followCamera->GetMovementDirectionXZ();
    fallbackForward.y = 0.0f;
    return SpatialMath::SafeNormalizeVector3(fallbackForward, Vector3(0.0f, 0.0f, 1.0f));
}

Vector3 ResolveCameraPlanarRight(Vector3 const &cameraPlanarForward) noexcept
{
    return SpatialMath::SafeNormalizeVector3(
        cameraPlanarForward.Cross(Vector3::UnitY),
        Vector3::UnitX
    );
}

float ResolveForwardAxisValue(Keyboard const &keyboard) noexcept
{
    float forwardAxisValue = 0.0f;
    if (keyboard.IsVirtualKeyDown('W'))
    {
        forwardAxisValue += 1.0f;
    }
    if (keyboard.IsVirtualKeyDown('S'))
    {
        forwardAxisValue -= 1.0f;
    }
    return forwardAxisValue;
}

float ResolveRightAxisValue(Keyboard const &keyboard) noexcept
{
    float rightAxisValue = 0.0f;
    if (keyboard.IsVirtualKeyDown('D'))
    {
        rightAxisValue += 1.0f;
    }
    if (keyboard.IsVirtualKeyDown('A'))
    {
        rightAxisValue -= 1.0f;
    }
    return rightAxisValue;
}

Vector3 ResolveMovementInputDirection(Keyboard const &keyboard, Vector3 const &planarForward, Vector3 const &planarRight) noexcept
{
    const float forwardAxisValue = ResolveForwardAxisValue(keyboard);
    const float rightAxisValue = ResolveRightAxisValue(keyboard);

    Vector3 movementInputDirection = planarForward * forwardAxisValue + planarRight * rightAxisValue;
    if (movementInputDirection.LengthSquared() > MinimumInputSquaredLength)
    {
        movementInputDirection.Normalize();
    }
    return movementInputDirection;
}
}

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
    Keyboard const &keyboard = context.Input.System->GetKeyboard();

    const Vector3 planarForward = ResolveCameraPlanarForwardToBall(
        GameplayWorld->FollowCameraForMovement,
        transform->Local.Position
    );
    const Vector3 planarRight = ResolveCameraPlanarRight(planarForward);
    const Vector3 inputDirection = ResolveMovementInputDirection(keyboard, planarForward, planarRight);

    if (inputDirection.LengthSquared() > MinimumInputSquaredLength)
    {
        Vector3 planarVelocity(velocity->LinearVelocity.x, 0.0f, velocity->LinearVelocity.z);
        const Vector3 targetPlanarVelocity = inputDirection * config.BallMaxHorizontalSpeed;
        Vector3 requiredPlanarDelta = targetPlanarVelocity - planarVelocity;
        const float maximumPlanarDelta = config.BallMoveAcceleration * deltaTime;
        const float requiredPlanarDeltaLength = requiredPlanarDelta.Length();
        if (requiredPlanarDeltaLength > maximumPlanarDelta && requiredPlanarDeltaLength > 1.0e-6f)
        {
            requiredPlanarDelta *= maximumPlanarDelta / requiredPlanarDeltaLength;
        }

        velocity->LinearVelocity.x += requiredPlanarDelta.x;
        velocity->LinearVelocity.z += requiredPlanarDelta.z;
    }

    // Gravity and jump (vertical axis is independent of input direction).
    const float groundedHeightThreshold = GameplayWorld->BallRadius + GroundedHeightEpsilon;
    const bool isGrounded = transform->Local.Position.y <= groundedHeightThreshold;
    if (keyboard.WasVirtualKeyPressed(VK_SPACE) && isGrounded)
    {
        velocity->LinearVelocity.y = config.BallJumpVelocity;
    }
    velocity->LinearVelocity.y -= config.BallGravityAcceleration * deltaTime;

    // Clamp planar speed to the configured maximum.
    Vector3 horizontalVelocity(velocity->LinearVelocity.x, 0.0f, velocity->LinearVelocity.z);
    const float horizontalSpeed = horizontalVelocity.Length();
    if (horizontalSpeed > config.BallMaxHorizontalSpeed)
    {
        const float clampScale = config.BallMaxHorizontalSpeed / horizontalSpeed;
        velocity->LinearVelocity.x *= clampScale;
        velocity->LinearVelocity.z *= clampScale;
    }

    // Apply planar drag (exponential damping that does not fight vertical motion).
    const float dragFactor = 1.0f - (std::min)(config.BallHorizontalDrag * deltaTime, MaximumDragPerFrame);
    velocity->LinearVelocity.x *= dragFactor;
    velocity->LinearVelocity.z *= dragFactor;
}
