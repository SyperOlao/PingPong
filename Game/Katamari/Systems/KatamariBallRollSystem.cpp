#include "Game/Katamari/Systems/KatamariBallRollSystem.h"

#include "Core/App/AppContext.h"
#include "Core/Gameplay/GameplayComponents.h"
#include "Core/Gameplay/Scene.h"
#include "Core/Math/SpatialMath.h"
#include "Game/Katamari/Data/KatamariWorldContext.h"

#include <algorithm>
#include <cmath>

using DirectX::SimpleMath::Quaternion;
using DirectX::SimpleMath::Vector3;

namespace
{
constexpr float MinimumHorizontalDistanceForRolling = 1.0e-4f;
constexpr float MinimumHorizontalSpeedForRolling = 1.0e-3f;
constexpr float RollDirectionSmoothingTimeSeconds = 0.075f;

Vector3 SmoothRollDirection(
    Vector3 const &currentDirection,
    Vector3 const &targetDirection,
    float const deltaTime
)
{
    if (deltaTime <= 0.0f)
    {
        return targetDirection;
    }

    if (currentDirection.Dot(targetDirection) < -0.35f)
    {
        return targetDirection;
    }

    const float alpha = 1.0f - std::exp(-deltaTime / RollDirectionSmoothingTimeSeconds);
    Vector3 blendedDirection = Vector3::Lerp(currentDirection, targetDirection, alpha);
    if (blendedDirection.LengthSquared() <= 1.0e-8f)
    {
        return targetDirection;
    }

    blendedDirection.Normalize();
    return blendedDirection;
}
}

KatamariBallRollSystem::KatamariBallRollSystem(KatamariWorldContext *const gameplayWorld) noexcept
    : GameplayWorld(gameplayWorld)
{
}

void KatamariBallRollSystem::Initialize(Scene &, AppContext &)
{
    RollOrientation = Quaternion::Identity;
    PreviousBallPosition = Vector3::Zero;
    SmoothedRollDirection = Vector3::UnitZ;
    HasPreviousBallPosition = false;
    HasSmoothedRollDirection = false;
}

void KatamariBallRollSystem::Update(Scene &scene, AppContext &, const float deltaTime)
{
    if (GameplayWorld == nullptr || GameplayWorld->Config == nullptr)
    {
        return;
    }

    const EntityId ballId = GameplayWorld->BallEntityId;
    TransformComponent *const transform = scene.TryGetTransformComponent(ballId);
    VelocityComponent const *const velocity = scene.TryGetVelocityComponent(ballId);
    if (transform == nullptr || velocity == nullptr)
    {
        return;
    }

    transform->Local.UseQuaternionRotation = true;
    transform->Local.RotationQuaternion = RollOrientation;

    const Vector3 currentBallPosition = transform->Local.Position;
    if (!HasPreviousBallPosition)
    {
        PreviousBallPosition = currentBallPosition;
        HasPreviousBallPosition = true;
        HasSmoothedRollDirection = false;
        transform->WorldMatrix = transform->Local.GetWorldMatrix();
        return;
    }

    Vector3 horizontalDisplacement = currentBallPosition - PreviousBallPosition;
    PreviousBallPosition = currentBallPosition;
    horizontalDisplacement.y = 0.0f;

    const bool isGrounded = transform->Local.Position.y <= GameplayWorld->BallRadius + 0.02f;
    const float horizontalDistance = horizontalDisplacement.Length();

    Vector3 horizontalVelocity = velocity->LinearVelocity;
    horizontalVelocity.y = 0.0f;
    const float horizontalSpeed = horizontalVelocity.Length();

    if (!isGrounded ||
        horizontalDistance < MinimumHorizontalDistanceForRolling ||
        horizontalSpeed < MinimumHorizontalSpeedForRolling)
    {
        HasSmoothedRollDirection = false;
        transform->WorldMatrix = transform->Local.GetWorldMatrix();
        return;
    }

    Vector3 movementDirection = horizontalVelocity / horizontalSpeed;
    if (movementDirection.Dot(horizontalDisplacement) < 0.0f)
    {
        movementDirection = -movementDirection;
    }

    if (!HasSmoothedRollDirection)
    {
        SmoothedRollDirection = movementDirection;
        HasSmoothedRollDirection = true;
    }
    else
    {
        SmoothedRollDirection = SmoothRollDirection(SmoothedRollDirection, movementDirection, deltaTime);
    }

    const Vector3 rollAxis = SpatialMath::SafeNormalizeVector3(
        Vector3::UnitY.Cross(SmoothedRollDirection),
        Vector3::Zero
    );
    if (rollAxis.LengthSquared() <= 1.0e-8f)
    {
        transform->WorldMatrix = transform->Local.GetWorldMatrix();
        return;
    }

    const float radius = (std::max)(GameplayWorld->BallRadius, 0.01f);
    const float visualScale = GameplayWorld->Config->BallVisualRollSpeedMultiplier;
    const float travelDistance = horizontalSpeed * deltaTime;
    const float angularDisplacement = (travelDistance / radius) * visualScale;
    if (angularDisplacement <= 1.0e-6f)
    {
        transform->WorldMatrix = transform->Local.GetWorldMatrix();
        return;
    }

    const Quaternion deltaRotation = Quaternion::CreateFromAxisAngle(rollAxis, angularDisplacement);
    RollOrientation = Quaternion::Concatenate(RollOrientation, deltaRotation);
    RollOrientation.Normalize();

    transform->Local.RotationQuaternion = RollOrientation;
    transform->WorldMatrix = transform->Local.GetWorldMatrix();
}
