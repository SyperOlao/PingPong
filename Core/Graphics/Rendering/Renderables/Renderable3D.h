// Created by SyperOlao on 25.03.2026.
#ifndef PINGPONG_RENDERABLE3D_H
#define PINGPONG_RENDERABLE3D_H

#include <cstdint>

#include "Core/Graphics/Rendering/Renderables/RenderGeometry3D.h"
#include "Core/Graphics/Rendering/Renderables/RenderMaterialParameters.h"
#include "Core/Graphics/Rendering/Renderables/RenderTransform3D.h"

using SceneObjectId = std::uint64_t;

struct Renderable3D final
{
    SceneObjectId SceneObjectIdValue{0};
    RenderTransform3D Transform{};
    RenderGeometry3D Geometry{};
    RenderMaterialParameters Material{};
    bool IsVisible{true};
};

#endif