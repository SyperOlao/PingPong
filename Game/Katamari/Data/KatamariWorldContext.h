#ifndef PINGPONG_KATAMARIWORLDCONTEXT_H
#define PINGPONG_KATAMARIWORLDCONTEXT_H

#include "Game/Katamari/Data/KatamariGameConfig.h"

#include "Core/Gameplay/EntityId.h"

#include <SimpleMath.h>

#include <random>
#include <unordered_map>

class FollowCamera;

struct KatamariPickupRecord final
{
    float AbsorbVolumeContribution{1.0f};
    float MinimumBallRadiusToAbsorb{0.0f};
    bool Absorbed{false};
};

struct KatamariWorldContext final
{
    EntityId BallEntityId{};
    float BallVolume{0.0f};
    float BallRadius{1.0f};
    float BallVisualRadius{1.0f};
    int CollectedCount{0};
    int TotalPickups{0};
    FollowCamera *FollowCameraForMovement{nullptr};
    KatamariGameConfig const *Config{nullptr};
    std::unordered_map<EntityId, KatamariPickupRecord> Pickups{};
    bool DebugDrawCollision{false};
    std::mt19937 Random{};
};

#endif
