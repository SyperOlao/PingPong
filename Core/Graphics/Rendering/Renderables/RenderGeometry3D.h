// Created by SyperOlao on 25.03.2026.
#ifndef PINGPONG_RENDERGEOMETRY3D_H
#define PINGPONG_RENDERGEOMETRY3D_H

#include <SimpleMath.h>

#include <memory>
#include <variant>
#include <vector>

#include "Core/Graphics/ModelAsset.h"

struct PrimitiveBoxGeometry3D final
{
};

struct PrimitiveSphereGeometry3D final
{
};

struct PrimitiveOrbitGeometry3D final
{
    std::vector<DirectX::SimpleMath::Vector3> Points{};
};

struct ModelGeometry3D final
{
    std::shared_ptr<const ModelAsset> Model{};
};

using RenderGeometry3D = std::variant<
    PrimitiveBoxGeometry3D,
    PrimitiveSphereGeometry3D,
    PrimitiveOrbitGeometry3D,
    ModelGeometry3D
>;

#endif