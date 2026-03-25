#ifndef PINGPONG_BOUNDINGSPHERE3D_H
#define PINGPONG_BOUNDINGSPHERE3D_H

#include <SimpleMath.h>

struct BoundingSphere3D final
{
    DirectX::SimpleMath::Vector3 Center{0.0f, 0.0f, 0.0f};
    float Radius{0.0f};

    [[nodiscard]] bool Intersects(const BoundingSphere3D &other) const noexcept
    {
        const float radiusSum = Radius + other.Radius;
        const DirectX::SimpleMath::Vector3 delta = other.Center - Center;
        return delta.LengthSquared() <= radiusSum * radiusSum;
    }
};

#endif
