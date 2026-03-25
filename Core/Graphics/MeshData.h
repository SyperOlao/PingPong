//
// Created by SyperOlao on 19.03.2026.
//

#ifndef PINGPONG_MESHDATA_H
#define PINGPONG_MESHDATA_H

#include "Vertex3D.h"

#include <cstdint>
#include <vector>

struct MeshData final {
    std::vector<Vertex3D> Vertices;
    std::vector<std::uint32_t> Indices;
};

#endif //PINGPONG_MESHDATA_H
