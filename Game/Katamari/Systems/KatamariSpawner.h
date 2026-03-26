#ifndef PINGPONG_KATAMARISPAWNER_H
#define PINGPONG_KATAMARISPAWNER_H

#include "Game/Katamari/Data/KatamariWorldContext.h"
#include "Game/Katamari/Data/PickupArchetype.h"

#include <vector>

struct AppContext;
class Scene;

class KatamariSpawner final
{
public:
    void SpawnPickups(
        Scene &scene,
        AppContext &context,
        KatamariWorldContext &world,
        std::vector<PickupArchetype> const &archetypes
    ) const;

private:
    [[nodiscard]] DirectX::SimpleMath::Vector3 SampleSpawnPosition(KatamariWorldContext &world) const;

    [[nodiscard]] bool IsSpawnPositionTooClose(
        Scene const &scene,
        KatamariWorldContext const &world,
        DirectX::SimpleMath::Vector3 const &candidate,
        float minimumSeparation
    ) const;
};

#endif
