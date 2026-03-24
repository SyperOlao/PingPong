//
// Created by SyperOlao on 25.03.2026.
//

#ifndef PINGPONG_SLIDERWIDGET_H
#define PINGPONG_SLIDERWIDGET_H
#include <string>

#include "Core/UI/Common/MouseState.h"
#include "Core/UI/Common/RectF.h"
#include "Core/UI/Common/UITheme.h"

class ShapeRenderer2D;

class SliderWidget final
{
public:
    void SetBounds(const RectF& bounds) noexcept;
    void SetLabel(std::string label);
    void SetRange(float minValue, float maxValue) noexcept;
    void SetValue(float value) noexcept;
    void SetTheme(const UITheme* theme) noexcept;

    void Update(const MouseState& mouse);
    void Render(ShapeRenderer2D& shapeRenderer) const;

    [[nodiscard]] float GetValue() const noexcept;
    [[nodiscard]] const RectF& GetBounds() const noexcept;
    [[nodiscard]] bool IsDragging() const noexcept;

private:
    [[nodiscard]] RectF GetTrackRect() const noexcept;
    [[nodiscard]] RectF GetHandleRect() const noexcept;
    [[nodiscard]] float GetNormalizedValue() const noexcept;
    [[nodiscard]] std::string BuildValueText() const;
    void SetValueFromMouse(float mouseX) noexcept;

private:
    RectF m_bounds{};
    std::string m_label{"Slider"};

    float m_minValue{0.0f};
    float m_maxValue{1.0f};
    float m_value{0.5f};

    bool m_isDragging{false};

    const UITheme* m_theme{nullptr};
};

#endif //PINGPONG_SLIDERWIDGET_H