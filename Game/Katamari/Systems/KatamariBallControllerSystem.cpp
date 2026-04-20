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

Vector3 ResolveCameraPlanarForward(FollowCamera *const followCamera) noexcept
{
    if (followCamera == nullptr)
    {
        return Vector3(0.0f, 0.0f, 1.0f);
    }

    Vector3 cameraForward = followCamera->GetMovementDirectionXZ();
    cameraForward.y = 0.0f;
    return SpatialMath::SafeNormalizeVector3(cameraForward, Vector3(0.0f, 0.0f, 1.0f));
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

    // Camera-relative planar basis. In DirectX SimpleMath right-handed space,
    // the screen-right world direction is forward x up (not up x forward).
    const Vector3 planarForward = ResolveCameraPlanarForward(GameplayWorld->FollowCameraForMovement);
    const Vector3 planarRight = SpatialMath::RightFromForwardAndWorldUp(planarForward, Vector3::UnitY);

    // Read WASD as a 2D input vector in camera space.
    Vector3 inputDirection = Vector3::Zero;
    if (keyboard.IsVirtualKeyDown('W'))
    {
        inputDirection += planarForward;
    }
    if (keyboard.IsVirtualKeyDown('S'))
    {
        inputDirection -= planarForward;
    }
    if (keyboard.IsVirtualKeyDown('D'))
    {
        inputDirection += planarRight;
    }
    if (keyboard.IsVirtualKeyDown('A'))
    {
        inputDirection -= planarRight;
    }

    if (inputDirection.LengthSquared() > MinimumInputSquaredLength)
    {
        inputDirection.Normalize();
        velocity->LinearVelocity.x += inputDirection.x * config.BallMoveAcceleration * deltaTime;
        velocity->LinearVelocity.z += inputDirection.z * config.BallMoveAcceleration * deltaTime;
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
