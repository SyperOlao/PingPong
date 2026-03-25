//
// Created by SyperOlao on 19.03.2026.
//

#ifndef PINGPONG_BUTTON_H
#define PINGPONG_BUTTON_H
#include <string>

#include "../Graphics/Color.h"
#include "../Graphics/Rendering/ShapeRenderer2D.h"

class BitmapFont;
class InputSystem;

struct ButtonStyle final
{
    Color NormalBackground{0.12f, 0.12f, 0.16f, 1.0f};
    Color SelectedBackground{0.20f, 0.20f, 0.28f, 1.0f};
    Color Accent{0.85f, 0.85f, 0.90f, 1.0f};
    Color Text{0.95f, 0.95f, 0.98f, 1.0f};
    Color SelectedText{1.0f, 1.0f, 1.0f, 1.0f};
    float BorderThickness{2.0f};
    float TextScale{1.0f};
};


class Button final
{
public:
    RectF Bounds{};
    std::string Label{};
    bool Enabled{true};

    [[nodiscard]] bool HandleKeyboard(const InputSystem& input, bool isSelected) const noexcept;
    void Draw(const ShapeRenderer2D& renderer, const BitmapFont& font, const ButtonStyle& style, bool isSelected) const;
};


#endif //PINGPONG_BUTTON_H