#ifndef PINGPONG_SCENECOLLISIONTYPES_H
#define PINGPONG_SCENECOLLISIONTYPES_H

#include "Core/Gameplay/EntityId.h"
#include "Core/Physics/Collision3D/CollisionContact3D.h"

struct CollisionPair final
{
    EntityId EntityA{0u};
    EntityId EntityB{0u};
    CollisionContact3D Contact{};
};

#endif
