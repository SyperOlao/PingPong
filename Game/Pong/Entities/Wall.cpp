//
// Created by SyperOlao on 18.03.2026.
//

#include "Wall.h"

void Wall::SetRect(const RectF& rect) noexcept
{
    m_rect = rect;
}

void Wall::SetColor(const Color& color) noexcept
{
    m_color = color;
}

const RectF& Wall::GetRect() const noexcept
{
    return m_rect;
}

void Wall::Render(const ShapeRenderer2D& renderer) const
{
    renderer.DrawFilledRect(
        m_rect.X,
        m_rect.Y,
        m_rect.Width,
        m_rect.Height,
        m_color
    );
}