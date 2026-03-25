#ifndef PINGPONG_CIRCLE2D_H
#define PINGPONG_CIRCLE2D_H

#include <SimpleMath.h>

struct Circle2D final
{
    DirectX::SimpleMath::Vector2 Center{0.0f, 0.0f};
    float Radius{0.0f};
};

#endif
