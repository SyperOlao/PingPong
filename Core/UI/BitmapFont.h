//
// Created by SyperOlao on 19.03.2026.
//

#ifndef PINGPONG_BITMAPFONT_H
#define PINGPONG_BITMAPFONT_H
#include <string>
#include <string_view>

#include "../Graphics/Color.h"

class ShapeRenderer2D;


class BitmapFont final {
public:
    [[nodiscard]] static float MeasureTextWidth(std::string_view text, float scale = 1.0f) noexcept;
    [[nodiscard]] static float MeasureTextHeight(float scale = 1.0f) noexcept;

    static void DrawString(
        const ShapeRenderer2D &renderer,
        float x,
        float y,
        std::string_view text,
        const Color &color,
        float scale = 1.0f
    );
private:
    static constexpr int GlyphWidth = 5;
    static constexpr int GlyphHeight = 7;
    static constexpr float PixelSize = 4.0f;
    static constexpr float GlyphSpacing = 1.0f;

    static void DrawGlyph(
        const ShapeRenderer2D& renderer,
        float x,
        float y,
        char character,
        const Color& color,
        float scale
    );
};


#endif //PINGPONG_BITMAPFONT_H
