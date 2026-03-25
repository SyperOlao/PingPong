#ifndef PINGPONG_AABB2D_H
#define PINGPONG_AABB2D_H

#include <SimpleMath.h>

struct AABB2D final
{
    DirectX::SimpleMath::Vector2 Min{0.0f, 0.0f};
    DirectX::SimpleMath::Vector2 Max{0.0f, 0.0f};

    AABB2D() = default;

    AABB2D(const DirectX::SimpleMath::Vector2 &minimumPoint, const DirectX::SimpleMath::Vector2 &maximumPoint) noexcept
        : Min(minimumPoint),
          Max(maximumPoint)
    {
    }

    [[nodiscard]] static AABB2D FromCenterExtents(
        const DirectX::SimpleMath::Vector2 &center,
        const DirectX::SimpleMath::Vector2 &halfExtents
    ) noexcept
    {
        return AABB2D(center - halfExtents, center + halfExtents);
    }

    [[nodiscard]] static AABB2D FromPositionSize(
        const DirectX::SimpleMath::Vector2 &position,
        const DirectX::SimpleMath::Vector2 &size
    ) noexcept
    {
        return AABB2D(position, position + size);
    }

    [[nodiscard]] DirectX::SimpleMath::Vector2 GetCenter() const noexcept
    {
        return (Min + Max) * 0.5f;
    }

    [[nodiscard]] DirectX::SimpleMath::Vector2 GetExtents() const noexcept
    {
        return (Max - Min) * 0.5f;
    }

    [[nodiscard]] DirectX::SimpleMath::Vector2 GetSize() const noexcept
    {
        return Max - Min;
    }

    [[nodiscard]] bool Contains(const DirectX::SimpleMath::Vector2 &point) const noexcept
    {
        return point.x >= Min.x && point.x <= Max.x && point.y >= Min.y && point.y <= Max.y;
    }

    [[nodiscard]] bool Intersects(const AABB2D &other) const noexcept
    {
        return Min.x <= other.Max.x && Max.x >= other.Min.x && Min.y <= other.Max.y && Max.y >= other.Min.y;
    }
};

#endif
