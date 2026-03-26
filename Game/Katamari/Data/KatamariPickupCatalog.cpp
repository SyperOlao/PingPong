#include "Game/Katamari/Data/KatamariPickupCatalog.h"

#include <utility>

std::vector<PickupArchetype> KatamariPickupCatalog::BuildDefaultPickupArchetypes()
{
    std::vector<PickupArchetype> archetypes{};

    PickupArchetype kakashi{};
    kakashi.DisplayName = "Kakashi";
    kakashi.ModelPath = std::filesystem::path("Game/Katamari/Assets/kakashi_free_fire/scene.gltf");
    kakashi.VisualUniformScale = 3.5f;
    kakashi.CollisionSphereRadius = 1.8f;
    kakashi.AbsorbVolumeContribution = 2.2f;
    kakashi.MinimumBallRadiusToAbsorb = 1.8f;
    archetypes.push_back(std::move(kakashi));

    PickupArchetype pipes{};
    pipes.DisplayName = "PipeOrgans";
    pipes.ModelPath = std::filesystem::path("Game/Katamari/Assets/pipe_organs/scene.gltf");
    pipes.VisualUniformScale = 0.45f;
    pipes.CollisionSphereRadius = 2.2f;
    pipes.AbsorbVolumeContribution = 4.5f;
    pipes.MinimumBallRadiusToAbsorb = 2.8f;
    archetypes.push_back(std::move(pipes));

    PickupArchetype harp{};
    harp.DisplayName = "Harp";
    harp.ModelPath = std::filesystem::path("Game/Katamari/Assets/harp-lyre/source/harpdone.fbx");
    harp.VisualUniformScale = 0.02f;
    harp.CollisionSphereRadius = 0.55f;
    harp.AbsorbVolumeContribution = 1.4f;
    harp.MinimumBallRadiusToAbsorb = 1.2f;
    archetypes.push_back(std::move(harp));

    return archetypes;
}
