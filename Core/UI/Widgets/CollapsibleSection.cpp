//
// Created by SyperOlao on 25.03.2026.
//

#include "CollapsibleSection.h"
#include <cassert>

#include "Core/Graphics2D/ShapeRenderer2D.h"
#include "Core/UI/BitmapFont.h"

void CollapsibleSection::SetBounds(const RectF& bounds) noexcept
{
    m_bounds = bounds;
}

void CollapsibleSection::SetTitle(std::string title)
{
    m_title = std::move(title);
}

void CollapsibleSection::SetExpanded(const bool isExpanded) noexcept
{
    m_isExpanded = isExpanded;
}

void CollapsibleSection::SetTheme(const UITheme* theme) noexcept
{
    m_theme = theme;
}

void CollapsibleSection::Update(const MouseState& mouse)
{
    assert(m_theme != nullptr);

    if (mouse.LeftPressed && m_bounds.Contains(mouse.X, mouse.Y))
    {
        m_isExpanded = !m_isExpanded;
    }
}

void CollapsibleSection::Render(ShapeRenderer2D& shapeRenderer) const
{
    assert(m_theme != nullptr);

    shapeRenderer.DrawFilledRect(
        m_bounds.X,
        m_bounds.Y,
        m_bounds.Width,
        m_bounds.Height,
        m_theme->PanelBorder
    );
    shapeRenderer.DrawFilledRect(
        m_bounds.X + 1.0f,
        m_bounds.Y + 1.0f,
        m_bounds.Width - 2.0f,
        m_bounds.Height - 2.0f,
        m_theme->HeaderBackground
    );

    const std::string prefix = m_isExpanded ? "- " : "+ ";
    BitmapFont::DrawString(
        shapeRenderer,
        m_bounds.X + 8.0f,
        m_bounds.Y + 5.0f,
        prefix + m_title,
        m_theme->HeaderText,
        0.42f
    );
}

bool CollapsibleSection::IsExpanded() const noexcept
{
    return m_isExpanded;
}

const RectF& CollapsibleSection::GetBounds() const noexcept
{
    return m_bounds;
}

const std::string& CollapsibleSection::GetTitle() const noexcept
{
    return m_title;
}