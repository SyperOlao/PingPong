#include "Game/Katamari/Systems/KatamariBallRollSystem.h"

#include "Core/App/AppContext.h"
#include "Core/Gameplay/GameplayComponents.h"
#include "Core/Gameplay/Scene.h"
#include "Core/Math/SpatialMath.h"
#include "Game/Katamari/Data/KatamariWorldContext.h"

#include <algorithm>

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Quaternion;
using DirectX::SimpleMath::Vector3;

namespace
{
constexpr float GroundedHeightEpsilon = 0.02f;
constexpr float MinimumHorizontalSpeedForRolling = 1.0e-3f;

Vector3 GetMovementMatrixRightDirection(Matrix const &movementMatrix) noexcept
{
    return Vector3(movementMatrix._11, movementMatrix._12, movementMatrix._13);
}

Vector3 GetMovementMatrixForwardDirection(Matrix const &movementMatrix) noexcept
{
    return Vector3(movementMatrix._31, movementMatrix._32, movementMatrix._33);
}

Vector3 ResolveRollAxisFromMovementDirection(Vector3 const &movementDirection) noexcept
{
    return SpatialMath::SafeNormalizeVector3(Vector3::UnitY.Cross(movementDirection), Vector3::Zero);
}
}

KatamariBallRollSystem::KatamariBallRollSystem(KatamariWorldContext *const gameplayWorld) noexcept
    : GameplayWorld(gameplayWorld)
{
}

void KatamariBallRollSystem::Initialize(Scene &, AppContext &)
{
    RollOrientation = Quaternion::Identity;
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

    Vector3 planarVelocity = velocity->LinearVelocity;
    planarVelocity.y = 0.0f;
    const float planarSpeed = planarVelocity.Length();
    const bool isGrounded = transform->Local.Position.y <= GameplayWorld->BallRadius + GroundedHeightEpsilon;
    if (!isGrounded || planarSpeed < MinimumHorizontalSpeedForRolling)
    {
        transform->WorldMatrix = transform->Local.GetWorldMatrix();
        return;
    }

    Vector3 movementDirection = planarVelocity / planarSpeed;
    float rollSpeed = planarSpeed;
    if (GameplayWorld->IsBallMovementMatrixValid)
    {
        const Vector3 matrixForwardDirection = GetMovementMatrixForwardDirection(GameplayWorld->BallMovementMatrix);
        const Vector3 matrixRightDirection = GetMovementMatrixRightDirection(GameplayWorld->BallMovementMatrix);

        const float forwardSpeed = planarVelocity.Dot(matrixForwardDirection);
        const float rightSpeed = planarVelocity.Dot(matrixRightDirection);
        Vector3 projectedVelocity = matrixForwardDirection * forwardSpeed + matrixRightDirection * rightSpeed;
        if (projectedVelocity.LengthSquared() > 1.0e-8f)
        {
            rollSpeed = projectedVelocity.Length();
            projectedVelocity.Normalize();
            movementDirection = projectedVelocity;
        }
    }

    const Vector3 rollAxis = ResolveRollAxisFromMovementDirection(movementDirection);
    if (rollAxis.LengthSquared() <= 1.0e-8f)
    {
        transform->WorldMatrix = transform->Local.GetWorldMatrix();
        return;
    }

    const float radius = (std::max)(GameplayWorld->BallRadius, 0.01f);
    const float visualScale = GameplayWorld->Config->BallVisualRollSpeedMultiplier;
    const float angularDisplacement = (rollSpeed * deltaTime / radius) * visualScale;
    if (angularDisplacement <= 1.0e-6f)
    {
        transform->WorldMatrix = transform->Local.GetWorldMatrix();
        return;
    }

    const Matrix currentRotation = Matrix::CreateFromQuaternion(RollOrientation);
    const Matrix deltaRotation = Matrix::CreateFromAxisAngle(rollAxis, angularDisplacement);
    RollOrientation = Quaternion::CreateFromRotationMatrix(currentRotation * deltaRotation);
    RollOrientation.Normalize();

    transform->Local.RotationQuaternion = RollOrientation;
    transform->WorldMatrix = transform->Local.GetWorldMatrix();
}
