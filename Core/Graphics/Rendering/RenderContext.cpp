//
// Created by SyperOlao on 25.03.2026.
//

#include "RenderContext.h"

#include "Core/Graphics/GraphicsDevice.h"

void RenderContext::Initialize(GraphicsDevice& graphics)
{
    m_graphics = &graphics;

    m_shapeRenderer2D.Initialize(graphics);
    m_primitiveRenderer3D.Initialize(graphics);
    m_modelRenderer.Initialize(graphics);
    m_sceneRenderer3D.Initialize(m_primitiveRenderer3D, m_modelRenderer);
}

ShapeRenderer2D& RenderContext::GetShapeRenderer2D() noexcept
{
    return m_shapeRenderer2D;
}

const ShapeRenderer2D& RenderContext::GetShapeRenderer2D() const noexcept
{
    return m_shapeRenderer2D;
}

PrimitiveRenderer3D& RenderContext::GetPrimitiveRenderer3D() noexcept
{
    return m_primitiveRenderer3D;
}

const PrimitiveRenderer3D& RenderContext::GetPrimitiveRenderer3D() const noexcept
{
    return m_primitiveRenderer3D;
}

SceneRenderer3D &RenderContext::GetSceneRenderer3D() noexcept
{
    return m_sceneRenderer3D;
}

const SceneRenderer3D &RenderContext::GetSceneRenderer3D() const noexcept
{
    return m_sceneRenderer3D;
}