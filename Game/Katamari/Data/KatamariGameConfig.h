#ifndef PINGPONG_KATAMARIGAMECONFIG_H
#define PINGPONG_KATAMARIGAMECONFIG_H

struct KatamariGameConfig final
{
    float PlayfieldHalfExtent{70.0f};
    float InitialBallRadius{2.0f};
    float BallMoveAcceleration{55.0f};
    float BallMaxHorizontalSpeed{42.0f};
    float BallHorizontalDrag{4.5f};
    float BallVisualRollSpeedMultiplier{1.0f};
    float AbsorbMinimumBallToPickupRadiusRatio{1.08f};
    float CollisionCellSize{6.0f};
    float WallThickness{2.0f};
    float WallHeight{18.0f};
    float FloorColliderHalfThickness{0.25f};
    unsigned int PickupSpawnCount{48u};
    unsigned int RandomSeed{1337u};
    unsigned int SpawnMaxAttemptsPerPickup{40u};
    float PickupSpawnMinSeparation{1.5f};
};

#endif
