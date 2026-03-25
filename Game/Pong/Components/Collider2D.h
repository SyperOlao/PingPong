//
// Created by SyperOlao on 17.03.2026.
//

#ifndef PINGPONG_COLLIDER2D_H
#define PINGPONG_COLLIDER2D_H
#include <SimpleMath.h>

#include "Core/Physics/AABB.h"
#include "Core/Math/Transform2D.h"

struct Collider2D final
{
    DirectX::SimpleMath::Vector2 HalfSize{0.0f, 0.0f};
    DirectX::SimpleMath::Vector2 Offset{0.0f, 0.0f};
    bool Enabled{true};

    [[nodiscard]] AABB GetWorldAABB(const Transform2D& transform) const noexcept
    {
        const auto scaledHalfSize = DirectX::SimpleMath::Vector2
        {
            HalfSize.x * transform.Scale.x,
            HalfSize.y * transform.Scale.y
        };

        const auto worldCenter = transform.Position + Offset + scaledHalfSize;
        return AABB::FromCenterExtents(worldCenter, scaledHalfSize);
    }
};
#endif //PINGPONG_COLLIDER2D_H