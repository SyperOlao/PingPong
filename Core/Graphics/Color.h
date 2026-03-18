//
// Created by SyperOlao on 17.03.2026.
//

#ifndef PINGPONG_COLOR_H
#define PINGPONG_COLOR_H
#include <array>

struct Color final
{
    float r{1.0f};
    float g{1.0f};
    float b{1.0f};
    float a{1.0f};

    constexpr Color() noexcept = default;

    constexpr Color(const float red, const float green, const float blue, const float alpha = 1.0f) noexcept
        : r(red)
        , g(green)
        , b(blue)
        , a(alpha)
    {
    }

    [[nodiscard]] constexpr std::array<float, 4> ToArray() const noexcept
    {
        return {r, g, b, a};
    }

    [[nodiscard]] static constexpr Color Black() noexcept
    {
        return {0.0f, 0.0f, 0.0f, 1.0f};
    }
};
#endif //PINGPONG_COLOR_H
