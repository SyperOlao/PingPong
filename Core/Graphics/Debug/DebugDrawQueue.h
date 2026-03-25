#ifndef PINGPONG_DEBUGDRAWQUEUE_H
#define PINGPONG_DEBUGDRAWQUEUE_H

#include <SimpleMath.h>

#include <vector>

#include "Core/Graphics/Vertex3D.h"
#include "Core/Physics/Collision3D/AxisAlignedBox3D.h"
#include "Core/Physics/Collision3D/BoundingSphere3D.h"

class PrimitiveRenderer3D;

class DebugDrawQueue final
{
public:
    void Clear() noexcept;

    void AddLineSegment(
        const DirectX::SimpleMath::Vector3 &startPosition,
        const DirectX::SimpleMath::Vector3 &endPosition,
        const DirectX::SimpleMath::Color &color
    );

    void AddWireSphere(
        const DirectX::SimpleMath::Vector3 &center,
        float radius,
        const DirectX::SimpleMath::Color &color,
        int ringSegmentCount = 24
    );

    void AddAxisAlignedBox(const AxisAlignedBox3D &box, const DirectX::SimpleMath::Color &color);

    void AddBoundingSphere(const BoundingSphere3D &sphere, const DirectX::SimpleMath::Color &color);

    void AddOrientedBox(
        const DirectX::SimpleMath::Matrix &world,
        const DirectX::SimpleMath::Vector3 &halfExtents,
        const DirectX::SimpleMath::Color &color
    );

    void AddWireBoxFromEightCorners(
        const DirectX::SimpleMath::Vector3 corners[8],
        const DirectX::SimpleMath::Color &color
    );

    void Flush(
        PrimitiveRenderer3D &primitiveRenderer,
        const DirectX::SimpleMath::Matrix &view,
        const DirectX::SimpleMath::Matrix &projection
    );

private:
    void AppendWireCircleInPlane(
        const DirectX::SimpleMath::Vector3 &center,
        float radius,
        const DirectX::SimpleMath::Vector3 &axisA,
        const DirectX::SimpleMath::Vector3 &axisB,
        const DirectX::SimpleMath::Color &color,
        int segmentCount
    );

    void AppendLineSegmentInternal(
        const DirectX::SimpleMath::Vector3 &startPosition,
        const DirectX::SimpleMath::Vector3 &endPosition,
        const DirectX::SimpleMath::Color &color
    );

private:
    std::vector<Vertex3D> m_lineVertices{};
    static constexpr std::size_t kMaximumLineVertices = 262144u;
};

#endif
