//
// Created by SyperOlao on 19.03.2026.
//

#include "Button.h"
#include "BitmapFont.h"
#include "../Input/InputSystem.h"

bool Button::HandleKeyboard(const InputSystem &input, const bool isSelected) const noexcept {
    if (!Enabled || !isSelected) {
        return false;
    }

    return input.GetKeyboard().WasKeyPressed(Key::Enter);
}

void Button::Draw(
    ShapeRenderer2D &renderer,
    const BitmapFont &font,
    const ButtonStyle &style,
    const bool isSelected
) const {
    const Color background = isSelected ? style.SelectedBackground : style.NormalBackground;
    const Color border = isSelected ? style.Accent : Color(0.35f, 0.35f, 0.42f, 1.0f);
    const Color textColor = isSelected ? style.SelectedText : style.Text;

    renderer.DrawFilledRect(Bounds, background);

    renderer.DrawFilledRect(Bounds.x, Bounds.y, Bounds.width, style.BorderThickness, border);
    renderer.DrawFilledRect(Bounds.x, Bounds.y + Bounds.height - style.BorderThickness, Bounds.width,
                            style.BorderThickness, border);
    renderer.DrawFilledRect(Bounds.x, Bounds.y, style.BorderThickness, Bounds.height, border);
    renderer.DrawFilledRect(Bounds.x + Bounds.width - style.BorderThickness, Bounds.y, style.BorderThickness,
                            Bounds.height, border);

    const float textWidth = font.MeasureTextWidth(Label, style.TextScale);
    const float textHeight = font.MeasureTextHeight(style.TextScale);

    const float textX = Bounds.x + (Bounds.width - textWidth) * 0.5f;
    const float textY = Bounds.y + (Bounds.height - textHeight) * 0.5f;

    font.DrawString(renderer, textX, textY, Label, textColor, style.TextScale);
}
