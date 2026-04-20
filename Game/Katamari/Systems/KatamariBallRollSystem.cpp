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

    const Vector3 movementDirection = planarVelocity / planarSpeed;
    const Vector3 rollAxis = SpatialMath::SafeNormalizeVector3(movementDirection.Cross(Vector3::UnitY), Vector3::Zero);
    if (rollAxis.LengthSquared() <= 1.0e-8f)
    {
        transform->WorldMatrix = transform->Local.GetWorldMatrix();
        return;
    }

    const float radius = (std::max)(GameplayWorld->BallRadius, 0.01f);
    const float visualScale = GameplayWorld->Config->BallVisualRollSpeedMultiplier;
    const float angularDisplacement = (planarSpeed * deltaTime / radius) * visualScale;
    if (angularDisplacement <= 1.0e-6f)
    {
        transform->WorldMatrix = transform->Local.GetWorldMatrix();
        return;
    }

    const Matrix currentRotation = Matrix::CreateFromQuaternion(RollOrientation);
    const Matrix deltaRotation = Matrix::CreateFromAxisAngle(rollAxis, angularDisplacement);
    RollOrientation = Quaternion::CreateFromRotationMatrix(deltaRotation * currentRotation);
    RollOrientation.Normalize();

    transform->Local.RotationQuaternion = RollOrientation;
    transform->WorldMatrix = transform->Local.GetWorldMatrix();
}
