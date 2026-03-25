#include "Core/Graphics/MeshGenerator.h"

#include <SimpleMath.h>

#include <cmath>

using DirectX::SimpleMath::Color;
using DirectX::SimpleMath::Vector3;


MeshData MeshGenerator::CreateBox(const float width, const float height, const float depth) {
    MeshData mesh{};

    const float hx = width * 0.5f;
    const float hy = height * 0.5f;
    const float hz = depth * 0.5f;

    constexpr auto color = Color(1.0f, 1.0f, 1.0f, 1.0f);

    mesh.Vertices =
    {
        Vertex3D(Vector3(-hx, -hy, -hz), color),
        Vertex3D(Vector3(-hx, +hy, -hz), color),
        Vertex3D(Vector3(+hx, +hy, -hz), color),
        Vertex3D(Vector3(+hx, -hy, -hz), color),

        Vertex3D(Vector3(-hx, -hy, +hz), color),
        Vertex3D(Vector3(-hx, +hy, +hz), color),
        Vertex3D(Vector3(+hx, +hy, +hz), color),
        Vertex3D(Vector3(+hx, -hy, +hz), color),
    };

    mesh.Indices =
    {
        0, 1, 2,  0, 2, 3,
        4, 6, 5,  4, 7, 6,
        4, 5, 1,  4, 1, 0,
        3, 2, 6,  3, 6, 7,
        1, 5, 6,  1, 6, 2,
        4, 0, 3,  4, 3, 7
    };

    return mesh;
}

MeshData MeshGenerator::CreateSphere(const float radius, const int slices, const int stacks) {
    MeshData mesh{};

    if (slices < 3 || stacks < 2) {
        return mesh;
    }

    constexpr auto color = Color(1.0f, 1.0f, 1.0f, 1.0f);

    for (int stack = 0; stack <= stacks; ++stack) {
        const float v = static_cast<float>(stack) / static_cast<float>(stacks);
        const float phi = DirectX::XM_PI * v;

        const float y = radius * std::cos(phi);
        const float ringRadius = radius * std::sin(phi);

        for (int slice = 0; slice <= slices; ++slice) {
            const float u = static_cast<float>(slice) / static_cast<float>(slices);
            const float theta = DirectX::XM_2PI * u;

            const float x = ringRadius * std::cos(theta);
            const float z = ringRadius * std::sin(theta);

            mesh.Vertices.emplace_back(Vector3(x, y, z), color);
        }
    }

    const int ringVertexCount = slices + 1;

    for (int stack = 0; stack < stacks; ++stack) {
        for (int slice = 0; slice < slices; ++slice) {
            const auto i0 = static_cast<std::uint32_t>(stack * ringVertexCount + slice);
            const std::uint32_t i1 = i0 + 1;
            const auto i2 = static_cast<std::uint32_t>((stack + 1) * ringVertexCount + slice);
            const std::uint32_t i3 = i2 + 1;

            mesh.Indices.push_back(i0);
            mesh.Indices.push_back(i2);
            mesh.Indices.push_back(i1);

            mesh.Indices.push_back(i1);
            mesh.Indices.push_back(i2);
            mesh.Indices.push_back(i3);
        }
    }

    return mesh;
}

namespace
{
    void AppendQuad(
        const Vector3 &vertexA,
        const Vector3 &vertexB,
        const Vector3 &vertexC,
        const Vector3 &vertexD,
        const Vector3 &normal,
        const Color &color,
        MeshLitData &mesh
    )
    {
        const std::uint32_t baseIndex = static_cast<std::uint32_t>(mesh.Vertices.size());

        MeshVertexLit3D vertex{};
        vertex.Normal = normal;
        vertex.Color = color;

        vertex.Position = vertexA;
        mesh.Vertices.push_back(vertex);
        vertex.Position = vertexB;
        mesh.Vertices.push_back(vertex);
        vertex.Position = vertexC;
        mesh.Vertices.push_back(vertex);
        vertex.Position = vertexD;
        mesh.Vertices.push_back(vertex);

        mesh.Indices.push_back(baseIndex + 0u);
        mesh.Indices.push_back(baseIndex + 1u);
        mesh.Indices.push_back(baseIndex + 2u);
        mesh.Indices.push_back(baseIndex + 0u);
        mesh.Indices.push_back(baseIndex + 2u);
        mesh.Indices.push_back(baseIndex + 3u);
    }
}

MeshLitData MeshGenerator::CreateBoxMeshLit(const float width, const float height, const float depth)
{
    MeshLitData mesh{};

    const float halfWidth = width * 0.5f;
    const float halfHeight = height * 0.5f;
    const float halfDepth = depth * 0.5f;

    constexpr Color whiteColor(1.0f, 1.0f, 1.0f, 1.0f);

    AppendQuad(
        Vector3(-halfWidth, -halfHeight, +halfDepth),
        Vector3(+halfWidth, -halfHeight, +halfDepth),
        Vector3(+halfWidth, +halfHeight, +halfDepth),
        Vector3(-halfWidth, +halfHeight, +halfDepth),
        Vector3(0.0f, 0.0f, 1.0f),
        whiteColor,
        mesh
    );

    AppendQuad(
        Vector3(-halfWidth, +halfHeight, -halfDepth),
        Vector3(+halfWidth, +halfHeight, -halfDepth),
        Vector3(+halfWidth, -halfHeight, -halfDepth),
        Vector3(-halfWidth, -halfHeight, -halfDepth),
        Vector3(0.0f, 0.0f, -1.0f),
        whiteColor,
        mesh
    );

    AppendQuad(
        Vector3(+halfWidth, -halfHeight, -halfDepth),
        Vector3(+halfWidth, +halfHeight, -halfDepth),
        Vector3(+halfWidth, +halfHeight, +halfDepth),
        Vector3(+halfWidth, -halfHeight, +halfDepth),
        Vector3(1.0f, 0.0f, 0.0f),
        whiteColor,
        mesh
    );

    AppendQuad(
        Vector3(-halfWidth, -halfHeight, +halfDepth),
        Vector3(-halfWidth, +halfHeight, +halfDepth),
        Vector3(-halfWidth, +halfHeight, -halfDepth),
        Vector3(-halfWidth, -halfHeight, -halfDepth),
        Vector3(-1.0f, 0.0f, 0.0f),
        whiteColor,
        mesh
    );

    AppendQuad(
        Vector3(-halfWidth, +halfHeight, +halfDepth),
        Vector3(+halfWidth, +halfHeight, +halfDepth),
        Vector3(+halfWidth, +halfHeight, -halfDepth),
        Vector3(-halfWidth, +halfHeight, -halfDepth),
        Vector3(0.0f, 1.0f, 0.0f),
        whiteColor,
        mesh
    );

    AppendQuad(
        Vector3(-halfWidth, -halfHeight, -halfDepth),
        Vector3(+halfWidth, -halfHeight, -halfDepth),
        Vector3(+halfWidth, -halfHeight, +halfDepth),
        Vector3(-halfWidth, -halfHeight, +halfDepth),
        Vector3(0.0f, -1.0f, 0.0f),
        whiteColor,
        mesh
    );

    return mesh;
}

MeshLitData MeshGenerator::CreateSphereMeshLit(const float radius, const int slices, const int stacks)
{
    MeshLitData mesh{};

    if (slices < 3 || stacks < 2)
    {
        return mesh;
    }

    constexpr Color whiteColor(1.0f, 1.0f, 1.0f, 1.0f);

    for (int stack = 0; stack <= stacks; ++stack)
    {
        const float vertical = static_cast<float>(stack) / static_cast<float>(stacks);
        const float phi = DirectX::XM_PI * vertical;

        const float y = radius * std::cos(phi);
        const float ringRadius = radius * std::sin(phi);

        for (int slice = 0; slice <= slices; ++slice)
        {
            const float horizontal = static_cast<float>(slice) / static_cast<float>(slices);
            const float theta = DirectX::XM_2PI * horizontal;

            const float x = ringRadius * std::cos(theta);
            const float z = ringRadius * std::sin(theta);

            const Vector3 position(x, y, z);
            Vector3 normal = position;
            normal.Normalize();

            MeshVertexLit3D vertex{};
            vertex.Position = position;
            vertex.Normal = normal;
            vertex.Color = whiteColor;
            mesh.Vertices.push_back(vertex);
        }
    }

    const int ringVertexCount = slices + 1;

    for (int stack = 0; stack < stacks; ++stack)
    {
        for (int slice = 0; slice < slices; ++slice)
        {
            const auto index0 = static_cast<std::uint32_t>(stack * ringVertexCount + slice);
            const std::uint32_t index1 = index0 + 1u;
            const auto index2 = static_cast<std::uint32_t>((stack + 1) * ringVertexCount + slice);
            const std::uint32_t index3 = index2 + 1u;

            mesh.Indices.push_back(index0);
            mesh.Indices.push_back(index2);
            mesh.Indices.push_back(index1);

            mesh.Indices.push_back(index1);
            mesh.Indices.push_back(index2);
            mesh.Indices.push_back(index3);
        }
    }

    return mesh;
}
