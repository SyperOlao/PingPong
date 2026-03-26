#ifndef PINGPONG_KATAMARILEVELSETUP_H
#define PINGPONG_KATAMARILEVELSETUP_H

#include "Game/Katamari/Data/KatamariWorldContext.h"

#include <vector>

struct PickupArchetype;

struct AppContext;
class Scene;

namespace KatamariLevelSetup
{
[[nodiscard]] float SphereVolumeFromRadius(float radius) noexcept;

void CreateStaticWorldCollision(Scene &scene, KatamariGameConfig const &config);

void CreatePlayerBall(Scene &scene, KatamariWorldContext &world);

void SpawnPlayerBallAndPickups(
    Scene &scene,
    AppContext &context,
    KatamariWorldContext &world,
    KatamariGameConfig const &config,
    std::vector<PickupArchetype> const &archetypes
);
}

#endif
