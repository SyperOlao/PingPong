//
// Created by SyperOlao on 25.03.2026.
//

#ifndef PINGPONG_KATAMARISPAWNER_H
#define PINGPONG_KATAMARISPAWNER_H
#include <vector>

#include "Core/Assets/AssetCache.h"
#include "Core/Graphics/Vertex3D.h"
#include "Game/Katamari/Data/KatamariGameConfig.h"
#include "Game/Katamari/Data/PickupArchetype.h"
#include "Game/Katamari/Entities/PickupItem.h"


class KatamariSpawner final
{
public:
    void Initialize(unsigned int seed);
    void SpawnInitialSet(
        const std::vector<PickupArchetype>& archetypes,
        std::vector<PickupItem>& outPickups,
        const KatamariGameConfig& config,
        AssetCache& assets
    );

private:
    [[nodiscard]] DirectX::SimpleMath::Vector3 GeneratePosition(const KatamariGameConfig& config);
};

#endif //PINGPONG_KATAMARISPAWNER_H