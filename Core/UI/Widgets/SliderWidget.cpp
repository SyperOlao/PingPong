//
// Created by SyperOlao on 25.03.2026.
//
#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "SliderWidget.h"

#include <algorithm>
#include <cassert>
#include <format>
#include "Core/Graphics2D/ShapeRenderer2D.h"
#include "Core/UI/BitmapFont.h"

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
    m_maxValue = std::max(minValue, maxValue);
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

    shapeRenderer.DrawFilledRect(m_bounds.X, m_bounds.Y, m_bounds.Width, m_bounds.Height, m_theme->PanelBorder);
    shapeRenderer.DrawFilledRect(
        m_bounds.X + 1.0f,
        m_bounds.Y + 1.0f,
        m_bounds.Width - 2.0f,
        m_bounds.Height - 2.0f,
        m_theme->SliderCard
    );

    constexpr float textScale = 0.38f;
    const std::string valueText = BuildValueText();
    const float valueWidth = BitmapFont::MeasureTextWidth(valueText, textScale);

    BitmapFont::DrawString(
        shapeRenderer,
        m_bounds.X + 8.0f,
        m_bounds.Y + 6.0f,
        m_label,
        m_theme->SliderLabel,
        textScale
    );

    BitmapFont::DrawString(
        shapeRenderer,
        m_bounds.X + m_bounds.Width - valueWidth - 8.0f,
        m_bounds.Y + 6.0f,
        valueText,
        m_theme->SliderValue,
        textScale
    );

    shapeRenderer.DrawFilledRect(trackRect.X, trackRect.Y, trackRect.Width, trackRect.Height, m_theme->SliderTrack);

    const float fillWidth = GetNormalizedValue() * trackRect.Width;
    shapeRenderer.DrawFilledRect(trackRect.X, trackRect.Y, fillWidth, trackRect.Height, m_theme->SliderFill);

    shapeRenderer.DrawFilledRect(handleRect.X, handleRect.Y, handleRect.Width, handleRect.Height, m_theme->SliderHandle);
    shapeRenderer.DrawFilledRect(
        handleRect.X + 3.0f,
        handleRect.Y + 3.0f,
        handleRect.Width - 6.0f,
        handleRect.Height - 6.0f,
        m_theme->SliderHandleAccent
    );
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
    const float horizontalPadding = 12.0f;
    const float trackY = m_bounds.Y + m_bounds.Height - 16.0f;

    return RectF{
        m_bounds.X + horizontalPadding,
        trackY,
        m_bounds.Width - horizontalPadding * 2.0f,
        m_theme->SliderTrackHeight
    };
}

RectF SliderWidget::GetHandleRect() const noexcept
{
    const RectF trackRect = GetTrackRect();
    const float t = GetNormalizedValue();

    const float handleX = trackRect.X + t * trackRect.Width - (m_theme->SliderHandleWidth * 0.5f);
    const float handleY = trackRect.Y - 5.0f;

    return RectF{
        handleX,
        handleY,
        m_theme->SliderHandleWidth,
        m_theme->SliderTrackHeight + 10.0f
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

std::string SliderWidget::BuildValueText() const
{
    return std::format("x{:.2f}", m_value);
}

void SliderWidget::SetValueFromMouse(const float mouseX) noexcept
{
    const RectF trackRect = GetTrackRect();
    const float t = std::clamp((mouseX - trackRect.X) / trackRect.Width, 0.0f, 1.0f);
    m_value = m_minValue + t * (m_maxValue - m_minValue);
}