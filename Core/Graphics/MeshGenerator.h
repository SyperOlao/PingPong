//
// Created by SyperOlao on 19.03.2026.
//

#ifndef PINGPONG_MESHGENERATOR_H
#define PINGPONG_MESHGENERATOR_H

#include "MeshData.h"
#include "MeshLitData.h"

class MeshGenerator final
{
public:
    [[nodiscard]] static MeshData CreateBox(float width, float height, float depth);

    [[nodiscard]] static MeshData CreateSphere(float radius, int slices, int stacks);

    [[nodiscard]] static MeshLitData CreateBoxMeshLit(float width, float height, float depth);

    [[nodiscard]] static MeshLitData CreateSphereMeshLit(float radius, int slices, int stacks);
};

#endif

