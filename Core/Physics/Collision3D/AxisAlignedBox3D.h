#ifndef PINGPONG_AXISALIGNEDBOX3D_H
#define PINGPONG_AXISALIGNEDBOX3D_H

#include <algorithm>
#include <SimpleMath.h>

struct AxisAlignedBox3D final
{
    DirectX::SimpleMath::Vector3 Min{0.0f, 0.0f, 0.0f};
    DirectX::SimpleMath::Vector3 Max{0.0f, 0.0f, 0.0f};

    [[nodiscard]] static AxisAlignedBox3D FromCenterExtents(
        const DirectX::SimpleMath::Vector3 &center,
        const DirectX::SimpleMath::Vector3 &halfExtents
    ) noexcept
    {
        AxisAlignedBox3D box{};
        box.Min = center - halfExtents;
        box.Max = center + halfExtents;
        return box;
    }

    [[nodiscard]] bool Contains(const DirectX::SimpleMath::Vector3 &point) const noexcept
    {
        return point.x >= Min.x && point.x <= Max.x && point.y >= Min.y && point.y <= Max.y && point.z >= Min.z &&
               point.z <= Max.z;
    }

    [[nodiscard]] DirectX::SimpleMath::Vector3 ClosestPointTo(const DirectX::SimpleMath::Vector3 &point) const noexcept
    {
        return DirectX::SimpleMath::Vector3(
            std::clamp(point.x, Min.x, Max.x),
            std::clamp(point.y, Min.y, Max.y),
            std::clamp(point.z, Min.z, Max.z)
        );
    }
};

#endif
