//
// Created by SyperOlao on 25.03.2026.
//

#include "SliderWidget.h"

#include <algorithm>
#include <cassert>

#include "Core/Graphics2D/ShapeRenderer2D.h"

void SliderWidget::SetBounds(const RectF& bounds) noexcept
{
    m_bounds = bounds;
}

void SliderWidget::SetLabel(std::string label)
{
    m_label = std::move(label);
}

void SliderWidget::SetRange(const float minValue, const float maxValue) noexcept
{
    m_minValue = minValue;
    m_maxValue = maxValue;
    m_value = std::clamp(m_value, m_minValue, m_maxValue);
}

void SliderWidget::SetValue(const float value) noexcept
{
    m_value = std::clamp(value, m_minValue, m_maxValue);
}

void SliderWidget::SetTheme(const UITheme* theme) noexcept
{
    m_theme = theme;
}

void SliderWidget::Update(const MouseState& mouse)
{
    assert(m_theme != nullptr);

    const RectF trackRect = GetTrackRect();
    const RectF handleRect = GetHandleRect();

    const bool hoveredTrack = trackRect.Contains(mouse.X, mouse.Y);
    const bool hoveredHandle = handleRect.Contains(mouse.X, mouse.Y);

    if (mouse.LeftPressed && (hoveredTrack || hoveredHandle))
    {
        m_isDragging = true;
        SetValueFromMouse(mouse.X);
    }

    if (m_isDragging && mouse.LeftDown)
    {
        SetValueFromMouse(mouse.X);
    }

    if (mouse.LeftReleased)
    {
        m_isDragging = false;
    }
}

void SliderWidget::Render(ShapeRenderer2D& shapeRenderer) const
{
    assert(m_theme != nullptr);

    const RectF trackRect = GetTrackRect();
    const RectF handleRect = GetHandleRect();

    shapeRenderer.DrawFilledRect(trackRect.X, trackRect.Y, trackRect.Width, trackRect.Height, m_theme->SliderTrack);

    const float fillWidth = GetNormalizedValue() * trackRect.Width;
    shapeRenderer.DrawFilledRect(trackRect.X, trackRect.Y, fillWidth, trackRect.Height, m_theme->SliderFill);

    shapeRenderer.DrawFilledRect(handleRect.X, handleRect.Y, handleRect.Width, handleRect.Height, m_theme->SliderHandle);
}

float SliderWidget::GetValue() const noexcept
{
    return m_value;
}

const RectF& SliderWidget::GetBounds() const noexcept
{
    return m_bounds;
}

bool SliderWidget::IsDragging() const noexcept
{
    return m_isDragging;
}

RectF SliderWidget::GetTrackRect() const noexcept
{
    const float trackY = m_bounds.Y + m_bounds.Height - 10.0f;
    return RectF{
        m_bounds.X,
        trackY,
        m_bounds.Width,
        m_theme->SliderTrackHeight
    };
}

RectF SliderWidget::GetHandleRect() const noexcept
{
    const RectF trackRect = GetTrackRect();
    const float t = GetNormalizedValue();

    const float handleX = trackRect.X + t * trackRect.Width - (m_theme->SliderHandleWidth * 0.5f);
    const float handleY = trackRect.Y - 4.0f;

    return RectF{
        handleX,
        handleY,
        m_theme->SliderHandleWidth,
        m_theme->SliderTrackHeight + 8.0f
    };
}

float SliderWidget::GetNormalizedValue() const noexcept
{
    const float range = m_maxValue - m_minValue;
    if (range <= 0.0001f)
    {
        return 0.0f;
    }

    return (m_value - m_minValue) / range;
}

void SliderWidget::SetValueFromMouse(const float mouseX) noexcept
{
    const RectF trackRect = GetTrackRect();
    const float t = std::clamp((mouseX - trackRect.X) / trackRect.Width, 0.0f, 1.0f);
    m_value = m_minValue + t * (m_maxValue - m_minValue);
}