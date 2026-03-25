// Created by SyperOlao on 25.03.2026.

#include "Core/Graphics/Rendering/Renderables/SceneRenderer3D.h"

#include "Core/Graphics/ModelRenderer.h"
#include "Core/Graphics/Rendering/PrimitiveRenderer3D.h"
#include "Core/Graphics/Rendering/Renderables/RenderGeometry3D.h"
#include "Core/Graphics/Rendering/Renderables/Renderable3D.h"

#include <type_traits>

void SceneRenderer3D::Initialize(
    PrimitiveRenderer3D &primitiveRenderer,
    ModelRenderer &modelRenderer
) noexcept
{
    m_primitiveRenderer = &primitiveRenderer;
    m_modelRenderer = &modelRenderer;
}

void SceneRenderer3D::RenderScene(const SceneRenderData3D &sceneRenderData) const
{
    if (m_primitiveRenderer == nullptr || m_modelRenderer == nullptr)
    {
        return;
    }

    const DirectX::SimpleMath::Matrix view = sceneRenderData.View;
    const DirectX::SimpleMath::Matrix projection = sceneRenderData.Projection;

    for (const Renderable3D &renderable : sceneRenderData.Renderables)
    {
        if (!renderable.IsVisible)
        {
            continue;
        }

        const DirectX::SimpleMath::Matrix world = renderable.Transform.GetWorldMatrix();

        std::visit(
            [&](const auto &geometry)
            {
                using GeometryType = std::decay_t<decltype(geometry)>;

                if constexpr (std::is_same_v<GeometryType, PrimitiveBoxGeometry3D>)
                {
                    m_primitiveRenderer->DrawBox(world, view, projection, renderable.Material.BaseColor);
                }
                else if constexpr (std::is_same_v<GeometryType, PrimitiveSphereGeometry3D>)
                {
                    m_primitiveRenderer->DrawSphere(world, view, projection, renderable.Material.BaseColor);
                }
                else if constexpr (std::is_same_v<GeometryType, PrimitiveOrbitGeometry3D>)
                {
                    m_primitiveRenderer->DrawOrbit(geometry.Points, view, projection, renderable.Material.BaseColor);
                }
                else if constexpr (std::is_same_v<GeometryType, ModelGeometry3D>)
                {
                    if (geometry.Model == nullptr)
                    {
                        return;
                    }

                    m_modelRenderer->DrawModel(*geometry.Model, world, view, projection, renderable.Material);
                }
            },
            renderable.Geometry
        );
    }
}
