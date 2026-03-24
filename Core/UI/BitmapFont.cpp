//
// Created by SyperOlao on 19.03.2026.
//

#include "BitmapFont.h"


#include <array>
#include <cctype>
#include <string_view>
#include <unordered_map>

#include "../Graphics2D/ShapeRenderer2D.h"


namespace
{
    using Glyph = std::array<std::string_view, 7>;

    const std::unordered_map<char, Glyph>& GetGlyphs()
    {
        static const std::unordered_map<char, Glyph> glyphs
        {
            {'0', {{"01110", "10001", "10011", "10101", "11001", "10001", "01110"}}},
            {'1', {{"00100", "01100", "00100", "00100", "00100", "00100", "01110"}}},
            {'2', {{"01110", "10001", "00001", "00010", "00100", "01000", "11111"}}},
            {'3', {{"11110", "00001", "00001", "01110", "00001", "00001", "11110"}}},
            {'4', {{"00010", "00110", "01010", "10010", "11111", "00010", "00010"}}},
            {'5', {{"11111", "10000", "10000", "11110", "00001", "00001", "11110"}}},
            {'6', {{"01110", "10000", "10000", "11110", "10001", "10001", "01110"}}},
            {'7', {{"11111", "00001", "00010", "00100", "01000", "01000", "01000"}}},
            {'8', {{"01110", "10001", "10001", "01110", "10001", "10001", "01110"}}},
            {'9', {{"01110", "10001", "10001", "01111", "00001", "00001", "01110"}}},

            {'A', {{"01110", "10001", "10001", "11111", "10001", "10001", "10001"}}},
            {'B', {{"11110", "10001", "10001", "11110", "10001", "10001", "11110"}}},
            {'C', {{"01111", "10000", "10000", "10000", "10000", "10000", "01111"}}},
            {'D', {{"11110", "10001", "10001", "10001", "10001", "10001", "11110"}}},
            {'E', {{"11111", "10000", "10000", "11110", "10000", "10000", "11111"}}},
            {'F', {{"11111", "10000", "10000", "11110", "10000", "10000", "10000"}}},
            {'G', {{"01111", "10000", "10000", "10011", "10001", "10001", "01111"}}},
            {'H', {{"10001", "10001", "10001", "11111", "10001", "10001", "10001"}}},
            {'I', {{"01110", "00100", "00100", "00100", "00100", "00100", "01110"}}},
            {'K', {{"10001", "10010", "10100", "11000", "10100", "10010", "10001"}}},
            {'L', {{"10000", "10000", "10000", "10000", "10000", "10000", "11111"}}},
            {'M', {{"10001", "11011", "10101", "10101", "10001", "10001", "10001"}}},
            {'N', {{"10001", "11001", "10101", "10011", "10001", "10001", "10001"}}},
            {'O', {{"01110", "10001", "10001", "10001", "10001", "10001", "01110"}}},
            {'P', {{"11110", "10001", "10001", "11110", "10000", "10000", "10000"}}},
            {'R', {{"11110", "10001", "10001", "11110", "10100", "10010", "10001"}}},
            {'S', {{"01111", "10000", "10000", "01110", "00001", "00001", "11110"}}},
            {'T', {{"11111", "00100", "00100", "00100", "00100", "00100", "00100"}}},
            {'U', {{"10001", "10001", "10001", "10001", "10001", "10001", "01110"}}},
            {'V', {{"10001", "10001", "10001", "10001", "10001", "01010", "00100"}}},
            {'W', {{"10001", "10001", "10001", "10101", "10101", "11011", "10001"}}},
            {'X', {{"10001", "10001", "01010", "00100", "01010", "10001", "10001"}}},
            {'Y', {{"10001", "10001", "01010", "00100", "00100", "00100", "00100"}}},
            {'Z', {{"11111", "00001", "00010", "00100", "01000", "10000", "11111"}}},
            {':', {{"00000", "00100", "00100", "00000", "00100", "00100", "00000"}}},
            {'-', {{"00000", "00000", "00000", "11111", "00000", "00000", "00000"}}},
            {'.', {{"00000", "00000", "00000", "00000", "00000", "00110", "00110"}}},
            {'J', {{"00111", "00010", "00010", "00010", "10010", "10010", "01100"}}},
            {'+', {{"00000", "00100", "00100", "11111", "00100", "00100", "00000"}}},
            {'/', {{"00001", "00010", "00100", "01000", "10000", "00000", "00000"}}},
            {'|', {{"00100", "00100", "00100", "00100", "00100", "00100", "00100"}}},
            {'(', {{"00010", "00100", "01000", "01000", "01000", "00100", "00010"}}},
            {')', {{"01000", "00100", "00010", "00010", "00010", "00100", "01000"}}},
            {' ', {{"00000", "00000", "00000", "00000", "00000", "00000", "00000"}}}
        };

        return glyphs;
    }
}

float BitmapFont::MeasureTextWidth(const std::string_view text, const float scale) noexcept
{
    if (text.empty())
    {
        return 0.0f;
    }

    const float glyphAdvance = (static_cast<float>(GlyphWidth) + GlyphSpacing) * PixelSize * scale;
    return glyphAdvance * static_cast<float>(text.size()) - (GlyphSpacing * PixelSize * scale);
}

float BitmapFont::MeasureTextHeight(const float scale) noexcept
{
    return static_cast<float>(GlyphHeight) * PixelSize * scale;
}

void BitmapFont::DrawString(
    const ShapeRenderer2D& renderer,
    float x,
    float y,
    const std::string_view text,
    const Color& color,
    const float scale
)
{
    const float glyphAdvance = (static_cast<float>(GlyphWidth) + GlyphSpacing) * PixelSize * scale;

    for (const char character : text)
    {
        DrawGlyph(renderer, x, y, character, color, scale);
        x += glyphAdvance;
    }
}

void BitmapFont::DrawGlyph(
    const ShapeRenderer2D& renderer,
    const float x,
    const float y,
    const char character,
    const Color& color,
    const float scale
)
{
    const auto& glyphs = GetGlyphs();
    const char upper = static_cast<char>(std::toupper(static_cast<unsigned char>(character)));

    const auto glyphIt = glyphs.find(upper);
    const auto fallbackIt = glyphs.find(' ');
    const Glyph& glyph = glyphIt != glyphs.end() ? glyphIt->second : fallbackIt->second;

    const float pixel = PixelSize * scale;

    for (int row = 0; row < GlyphHeight; ++row)
    {
        for (int column = 0; column < GlyphWidth; ++column)
        {
            if (glyph[static_cast<std::size_t>(row)][static_cast<std::size_t>(column)] != '1')
            {
                continue;
            }

            renderer.DrawFilledRect(
                x + static_cast<float>(column) * pixel,
                y + static_cast<float>(row) * pixel,
                pixel,
                pixel,
                color
            );
        }
    }
}