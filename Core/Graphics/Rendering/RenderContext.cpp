//
// Created by SyperOlao on 25.03.2026.
//

#include "RenderContext.h"
#include <stdexcept>

#include "Core/Graphics/GraphicsDevice.h"

void RenderContext::Initialize(GraphicsDevice& graphics)
{
    m_graphics = &graphics;

    m_shapeRenderer2D.Initialize(graphics);
    m_primitiveRenderer3D.Initialize(graphics);
}

void RenderContext::BindDefaultTargets() const
{
    if (m_graphics == nullptr)
    {
        throw std::logic_error("RenderContext::BindDefaultTargets called before Initialize.");
    }

    m_graphics->BindBackBufferRenderTargetOnly();
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