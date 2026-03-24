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
    const ShapeRenderer2D &renderer,
    const BitmapFont &font,
    const ButtonStyle &style,
    const bool isSelected
) const {
    const Color background = isSelected ? style.SelectedBackground : style.NormalBackground;
    const Color border = isSelected ? style.Accent : Color(0.35f, 0.35f, 0.42f, 1.0f);
    const Color textColor = isSelected ? style.SelectedText : style.Text;

    renderer.DrawFilledRect(Bounds, background);

    renderer.DrawFilledRect(Bounds.X, Bounds.Y, Bounds.Width, style.BorderThickness, border);
    renderer.DrawFilledRect(Bounds.X, Bounds.Y + Bounds.Height - style.BorderThickness, Bounds.Width,
                            style.BorderThickness, border);
    renderer.DrawFilledRect(Bounds.X, Bounds.Y, style.BorderThickness, Bounds.Height, border);
    renderer.DrawFilledRect(Bounds.X + Bounds.Width - style.BorderThickness, Bounds.Y, style.BorderThickness,
                            Bounds.Height, border);

    const float textWidth = BitmapFont::MeasureTextWidth(Label, style.TextScale);
    const float textHeight = BitmapFont::MeasureTextHeight(style.TextScale);

    const float textX = Bounds.X + (Bounds.Width - textWidth) * 0.5f;
    const float textY = Bounds.Y + (Bounds.Height - textHeight) * 0.5f;

    BitmapFont::DrawString(renderer, textX, textY, Label, textColor, style.TextScale);
}
