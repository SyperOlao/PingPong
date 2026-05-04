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
using DirectX::SimpleMath::Matrix;

namespace
{
constexpr float GroundedHeightEpsilon = 0.02f;
constexpr float MinimumInputSquaredLength = 1.0e-8f;
constexpr float MaximumDragPerFrame = 0.95f;
constexpr float MinimumDirectionSquaredLength = 1.0e-8f;

Vector3 ResolveCameraPlanarForward(
    FollowCamera *const followCamera,
    Vector3 const &ballWorldPosition
) noexcept
{
    if (followCamera == nullptr)
    {
        return Vector3(0.0f, 0.0f, 1.0f);
    }

    Vector3 cameraForwardDirection = followCamera->GetMovementDirectionXZ();
    cameraForwardDirection.y = 0.0f;
    if (cameraForwardDirection.LengthSquared() > MinimumDirectionSquaredLength)
    {
        cameraForwardDirection.Normalize();
        return cameraForwardDirection;
    }

    Vector3 cameraToBallDirection = ballWorldPosition - followCamera->GetPosition();
    cameraToBallDirection.y = 0.0f;
    if (cameraToBallDirection.LengthSquared() > MinimumDirectionSquaredLength)
    {
        cameraToBallDirection.Normalize();
        return cameraToBallDirection;
    }

    return Vector3(0.0f, 0.0f, 1.0f);
}

Matrix ResolvePlanarMovementMatrix(
    FollowCamera *const followCamera,
    Vector3 const &ballWorldPosition
) noexcept
{
    const Vector3 forwardDirection = ResolveCameraPlanarForward(followCamera, ballWorldPosition);
    const Vector3 rightDirection = -SpatialMath::RightFromForwardAndWorldUp(forwardDirection, Vector3::UnitY);

    Matrix movementMatrix = Matrix::Identity;
    movementMatrix._11 = rightDirection.x;
    movementMatrix._12 = rightDirection.y;
    movementMatrix._13 = rightDirection.z;

    movementMatrix._21 = Vector3::UnitY.x;
    movementMatrix._22 = Vector3::UnitY.y;
    movementMatrix._23 = Vector3::UnitY.z;

    movementMatrix._31 = forwardDirection.x;
    movementMatrix._32 = forwardDirection.y;
    movementMatrix._33 = forwardDirection.z;
    return movementMatrix;
}

Vector3 GetMovementMatrixRightDirection(Matrix const &movementMatrix) noexcept
{
    return Vector3(movementMatrix._11, movementMatrix._12, movementMatrix._13);
}

Vector3 GetMovementMatrixForwardDirection(Matrix const &movementMatrix) noexcept
{
    return Vector3(movementMatrix._31, movementMatrix._32, movementMatrix._33);
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

Vector3 ResolveMovementInputDirection(Keyboard const &keyboard, Matrix const &movementMatrix) noexcept
{
    const float forwardAxisValue = ResolveForwardAxisValue(keyboard);
    const float rightAxisValue = ResolveRightAxisValue(keyboard);

    const Vector3 forwardDirection = GetMovementMatrixForwardDirection(movementMatrix);
    const Vector3 rightDirection = GetMovementMatrixRightDirection(movementMatrix);

    Vector3 movementInputDirection = forwardDirection * forwardAxisValue;
    movementInputDirection += rightDirection * rightAxisValue;
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

    const Matrix movementMatrix = ResolvePlanarMovementMatrix(
        GameplayWorld->FollowCameraForMovement,
        transform->Local.Position
    );
    GameplayWorld->BallMovementMatrix = movementMatrix;
    GameplayWorld->IsBallMovementMatrixValid = true;
    const Vector3 inputDirection = ResolveMovementInputDirection(keyboard, movementMatrix);

    if (inputDirection.LengthSquared() > MinimumInputSquaredLength)
    {
        const Vector3 targetPlanarVelocity = inputDirection * config.BallMaxHorizontalSpeed;
        Vector3 planarVelocity(velocity->LinearVelocity.x, 0.0f, velocity->LinearVelocity.z);
        Vector3 requiredPlanarVelocityDelta = targetPlanarVelocity - planarVelocity;

        const float maximumPlanarVelocityDelta = config.BallMoveAcceleration * deltaTime;
        const float requiredPlanarVelocityDeltaLength = requiredPlanarVelocityDelta.Length();
        if (requiredPlanarVelocityDeltaLength > maximumPlanarVelocityDelta
            && requiredPlanarVelocityDeltaLength > MinimumDirectionSquaredLength)
        {
            requiredPlanarVelocityDelta *= maximumPlanarVelocityDelta / requiredPlanarVelocityDeltaLength;
        }

        velocity->LinearVelocity.x += requiredPlanarVelocityDelta.x;
        velocity->LinearVelocity.z += requiredPlanarVelocityDelta.z;
    }

    // Gravity and jump (vertical axis is independent of input direction).
    const float groundedHeightThreshold = GameplayWorld->BallRadius + GroundedHeightEpsilon;
    const bool isGrounded = transform->Local.Position.y <= groundedHeightThreshold;
    if (keyboard.WasVirtualKeyPressed(VK_SPACE) && isGrounded)
    {
        velocity->LinearVelocity.y = config.BallJumpVelocity;
    }
    velocity->LinearVelocity.y -= config.BallGravityAcceleration * deltaTime;

    Vector3 horizontalVelocity(velocity->LinearVelocity.x, 0.0f, velocity->LinearVelocity.z);
    const float horizontalSpeed = horizontalVelocity.Length();
    if (horizontalSpeed > config.BallMaxHorizontalSpeed)
    {
        const float clampScale = config.BallMaxHorizontalSpeed / horizontalSpeed;
        velocity->LinearVelocity.x *= clampScale;
        velocity->LinearVelocity.z *= clampScale;
    }

    const float dragFactor = 1.0f - (std::min)(config.BallHorizontalDrag * deltaTime, MaximumDragPerFrame);
    velocity->LinearVelocity.x *= dragFactor;
    velocity->LinearVelocity.z *= dragFactor;
}
