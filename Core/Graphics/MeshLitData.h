#ifndef PINGPONG_MESHLITDATA_H
#define PINGPONG_MESHLITDATA_H

#include <SimpleMath.h>

#include <cstdint>
#include <vector>

struct MeshVertexLit3D final
{
    DirectX::SimpleMath::Vector3 Position{};
    DirectX::SimpleMath::Vector3 Normal{};
    DirectX::SimpleMath::Color Color{1.0f, 1.0f, 1.0f, 1.0f};
};

struct MeshLitData final
{
    std::vector<MeshVertexLit3D> Vertices{};
    std::vector<std::uint32_t> Indices{};
};

#endif
