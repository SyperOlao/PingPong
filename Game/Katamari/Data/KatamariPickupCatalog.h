#ifndef PINGPONG_KATAMARIPICKUPCATALOG_H
#define PINGPONG_KATAMARIPICKUPCATALOG_H

#include "Game/Katamari/Data/PickupArchetype.h"

#include <vector>

class KatamariPickupCatalog final
{
public:
    [[nodiscard]] static std::vector<PickupArchetype> BuildDefaultPickupArchetypes();
};

#endif
