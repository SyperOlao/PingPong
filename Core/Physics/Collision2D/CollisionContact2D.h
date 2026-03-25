#ifndef PINGPONG_COLLISIONCONTACT2D_H
#define PINGPONG_COLLISIONCONTACT2D_H

#include <SimpleMath.h>

struct CollisionContact2D final
{
    bool HasOverlap{false};
    DirectX::SimpleMath::Vector2 Normal{0.0f, 0.0f};
    float PenetrationDepth{0.0f};
};

#endif
