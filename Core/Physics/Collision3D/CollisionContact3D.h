#ifndef PINGPONG_COLLISIONCONTACT3D_H
#define PINGPONG_COLLISIONCONTACT3D_H

#include <SimpleMath.h>

struct CollisionContact3D final
{
    bool HasOverlap{false};
    DirectX::SimpleMath::Vector3 Normal{0.0f, 1.0f, 0.0f};
    float PenetrationDepth{0.0f};
};

using SphereContact = CollisionContact3D;

#endif
