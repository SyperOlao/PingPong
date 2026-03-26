#ifndef PINGPONG_PICKUPARCHETYPE_H
#define PINGPONG_PICKUPARCHETYPE_H

#include <filesystem>
#include <string>

struct PickupArchetype final
{
    std::string DisplayName{};
    std::filesystem::path ModelPath{};
    float VisualUniformScale{1.0f};
    float CollisionSphereRadius{0.0f};
    float AbsorbVolumeContribution{1.0f};
    float MinimumBallRadiusToAbsorb{0.35f};
};

#endif
