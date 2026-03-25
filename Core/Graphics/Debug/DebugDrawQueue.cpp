#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "Core/Graphics/Debug/DebugDrawQueue.h"

#include "Core/Graphics/Rendering/PrimitiveRenderer3D.h"

#include <array>
#include <cmath>
#include <numbers>

using DirectX::SimpleMath::Color;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

namespace
{
    constexpr float kMinimumRadius = 1.0e-4f;

    constexpr std::array<int, 24> kBoxEdgePairs =
    {
        0, 1,
        1, 2,
        2, 3,
        3, 0,
        4, 5,
        5, 6,
        6, 7,
        7, 4,
        0, 4,
        1, 5,
        2, 6,
        3, 7
    };
}

void DebugDrawQueue::Clear() noexcept
{
    m_lineVertices.clear();
}

void DebugDrawQueue::AppendLineSegmentInternal(
    const Vector3 &startPosition,
    const Vector3 &endPosition,
    const Color &color
)
{
    if (m_lineVertices.size() + 2u > kMaximumLineVertices)
    {
        return;
    }

    m_lineVertices.emplace_back(startPosition, color);
    m_lineVertices.emplace_back(endPosition, color);
}

void DebugDrawQueue::AddLineSegment(
    const Vector3 &startPosition,
    const Vector3 &endPosition,
    const Color &color
)
{
    AppendLineSegmentInternal(startPosition, endPosition, color);
}

void DebugDrawQueue::AppendWireCircleInPlane(
    const Vector3 &center,
    const float radius,
    const Vector3 &axisA,
    const Vector3 &axisB,
    const Color &color,
    const int segmentCount
)
{
    if (segmentCount < 3)
    {
        return;
    }

    const float clampedRadius = std::max(radius, kMinimumRadius);
    const float twoPi = 2.0f * std::numbers::pi_v<float>;

    for (int segmentIndex = 0; segmentIndex < segmentCount; ++segmentIndex)
    {
        const float angle0 = twoPi * static_cast<float>(segmentIndex) / static_cast<float>(segmentCount);
        const float angle1 = twoPi * static_cast<float>(segmentIndex + 1) / static_cast<float>(segmentCount);

        const Vector3 point0 =
            center + axisA * (std::cos(angle0) * clampedRadius) + axisB * (std::sin(angle0) * clampedRadius);
        const Vector3 point1 =
            center + axisA * (std::cos(angle1) * clampedRadius) + axisB * (std::sin(angle1) * clampedRadius);

        AppendLineSegmentInternal(point0, point1, color);
    }
}

void DebugDrawQueue::AddWireSphere(
    const Vector3 &center,
    const float radius,
    const Color &color,
    const int ringSegmentCount
)
{
    const int clampedSegments = std::max(ringSegmentCount, 3);

    const Vector3 axisX(1.0f, 0.0f, 0.0f);
    const Vector3 axisY(0.0f, 1.0f, 0.0f);
    const Vector3 axisZ(0.0f, 0.0f, 1.0f);

    AppendWireCircleInPlane(center, radius, axisX, axisY, color, clampedSegments);
    AppendWireCircleInPlane(center, radius, axisX, axisZ, color, clampedSegments);
    AppendWireCircleInPlane(center, radius, axisY, axisZ, color, clampedSegments);
}

void DebugDrawQueue::AddAxisAlignedBox(const AxisAlignedBox3D &box, const Color &color)
{
    Vector3 corners[8]{};
    corners[0] = Vector3(box.Min.x, box.Min.y, box.Min.z);
    corners[1] = Vector3(box.Max.x, box.Min.y, box.Min.z);
    corners[2] = Vector3(box.Max.x, box.Max.y, box.Min.z);
    corners[3] = Vector3(box.Min.x, box.Max.y, box.Min.z);
    corners[4] = Vector3(box.Min.x, box.Min.y, box.Max.z);
    corners[5] = Vector3(box.Max.x, box.Min.y, box.Max.z);
    corners[6] = Vector3(box.Max.x, box.Max.y, box.Max.z);
    corners[7] = Vector3(box.Min.x, box.Max.y, box.Max.z);

    AddWireBoxFromEightCorners(corners, color);
}

void DebugDrawQueue::AddBoundingSphere(const BoundingSphere3D &sphere, const Color &color)
{
    AddWireSphere(sphere.Center, sphere.Radius, color, 24);
}

void DebugDrawQueue::AddOrientedBox(
    const Matrix &world,
    const Vector3 &halfExtents,
    const Color &color
)
{
    if (halfExtents.x <= 0.0f || halfExtents.y <= 0.0f || halfExtents.z <= 0.0f)
    {
        return;
    }

    Vector3 corners[8]{};
    corners[0] = Vector3::Transform(Vector3(-halfExtents.x, -halfExtents.y, -halfExtents.z), world);
    corners[1] = Vector3::Transform(Vector3(+halfExtents.x, -halfExtents.y, -halfExtents.z), world);
    corners[2] = Vector3::Transform(Vector3(+halfExtents.x, +halfExtents.y, -halfExtents.z), world);
    corners[3] = Vector3::Transform(Vector3(-halfExtents.x, +halfExtents.y, -halfExtents.z), world);
    corners[4] = Vector3::Transform(Vector3(-halfExtents.x, -halfExtents.y, +halfExtents.z), world);
    corners[5] = Vector3::Transform(Vector3(+halfExtents.x, -halfExtents.y, +halfExtents.z), world);
    corners[6] = Vector3::Transform(Vector3(+halfExtents.x, +halfExtents.y, +halfExtents.z), world);
    corners[7] = Vector3::Transform(Vector3(-halfExtents.x, +halfExtents.y, +halfExtents.z), world);

    AddWireBoxFromEightCorners(corners, color);
}

void DebugDrawQueue::AddWireBoxFromEightCorners(
    const Vector3 corners[8],
    const Color &color
)
{
    for (size_t edgeIndex = 0u; edgeIndex < 12u; ++edgeIndex)
    {
        const int cornerA = kBoxEdgePairs[edgeIndex * 2u];
        const int cornerB = kBoxEdgePairs[edgeIndex * 2u + 1u];
        AppendLineSegmentInternal(corners[cornerA], corners[cornerB], color);
    }
}

void DebugDrawQueue::Flush(
    PrimitiveRenderer3D &primitiveRenderer,
    const Matrix &view,
    const Matrix &projection
)
{
    if (m_lineVertices.empty())
    {
        return;
    }

    primitiveRenderer.DrawLineList(m_lineVertices.data(), static_cast<UINT>(m_lineVertices.size()), view, projection);
    m_lineVertices.clear();
}
