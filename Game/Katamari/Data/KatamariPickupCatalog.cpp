#include "Game/Katamari/Data/KatamariPickupCatalog.h"

#include <utility>

std::vector<PickupArchetype> KatamariPickupCatalog::BuildDefaultPickupArchetypes()
{
    std::vector<PickupArchetype> archetypes{};

    PickupArchetype tank{};
    tank.DisplayName = "Tank";
    tank.ModelPath = std::filesystem::path("Game/Katamari/Assets/tank/Tank.fbx");
    tank.VisualUniformScale = 0.045f;
    tank.CollisionSphereRadius = 0.0f;
    tank.AbsorbVolumeContribution = 2.2f;
    tank.MinimumBallRadiusToAbsorb = 1.8f;
    archetypes.push_back(std::move(tank));

    PickupArchetype pipes{};
    pipes.DisplayName = "PipeOrgans";
    pipes.ModelPath = std::filesystem::path("Game/Katamari/Assets/pipe_organs/scene.gltf");
    pipes.VisualUniformScale = 0.45f;
    pipes.CollisionSphereRadius = 0.0f;
    pipes.AbsorbVolumeContribution = 4.5f;
    pipes.MinimumBallRadiusToAbsorb = 2.8f;
    archetypes.push_back(std::move(pipes));

    PickupArchetype harp{};
    harp.DisplayName = "Harp";
    harp.ModelPath = std::filesystem::path("Game/Katamari/Assets/harp-lyre/source/harpdone.fbx");
    harp.VisualUniformScale = 0.02f;
    harp.CollisionSphereRadius = 0.0f;
    harp.AbsorbVolumeContribution = 1.4f;
    harp.MinimumBallRadiusToAbsorb = 1.2f;
    archetypes.push_back(std::move(harp));

    return archetypes;
}
