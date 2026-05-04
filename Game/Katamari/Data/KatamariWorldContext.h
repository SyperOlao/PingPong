#ifndef PINGPONG_KATAMARIWORLDCONTEXT_H
#define PINGPONG_KATAMARIWORLDCONTEXT_H

#include "Game/Katamari/Data/KatamariGameConfig.h"
#include "Game/Katamari/KatamariTypes.h"

#include "Core/Gameplay/EntityId.h"
#include "Core/Graphics/Rendering/Renderables/RenderMaterialParameters.h"

#include <SimpleMath.h>

#include <random>
#include <unordered_map>
#include <vector>

class FollowCamera;

struct KatamariPickupRecord final
{
    float AbsorbVolumeContribution{1.0f};
    float MinimumBallRadiusToAbsorb{0.0f};
    bool Absorbed{false};
};

struct KatamariStaticObstacleRecord final
{
    EntityId Entity{};
    KatamariObstacleShape Shape{KatamariObstacleShape::Cube};
    RenderMaterialParameters Material{};
};

struct KatamariWorldContext final
{
    EntityId BallEntityId{};
    float BallVolume{0.0f};
    float BallRadius{1.0f};
    float BallVisualRadius{1.0f};
    float BallMeshReferenceRadius{1.0f};
    int CollectedCount{0};
    int TotalPickups{0};
    FollowCamera *FollowCameraForMovement{nullptr};
    KatamariGameConfig const *Config{nullptr};
    DirectX::SimpleMath::Matrix BallMovementMatrix{DirectX::SimpleMath::Matrix::Identity};
    bool IsBallMovementMatrixValid{false};
    std::unordered_map<EntityId, KatamariPickupRecord> Pickups{};
    std::vector<KatamariStaticObstacleRecord> StaticObstacles{};
    bool DebugDrawCollision{false};
    std::mt19937 Random{};
};

#endif
