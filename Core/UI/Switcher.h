//
// Created by SyperOlao on 19.03.2026.
//

#ifndef PINGPONG_SWITCHER_H
#define PINGPONG_SWITCHER_H

#include <algorithm>
#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

#include "Core/Input/InputSystem.h"

class Switcher final
{
public:
    Switcher() = default;

    Switcher(
        const float x,
        const float y,
        const float width,
        const float height,
        std::string label,
        std::initializer_list<std::string> options
    )
        : m_x(x)
        , m_y(y)
        , m_width(width)
        , m_height(height)
        , m_label(std::move(label))
        , m_options(options.begin(), options.end())
    {
    }

    [[nodiscard]] bool HandleKeyboard(const InputSystem& input)
    {
        if (m_options.empty())
        {
            return false;
        }

        if (input.GetKeyboard().WasKeyPressed(Key::A) || input.GetKeyboard().WasKeyPressed(Key::Left))
        {
            m_selectedIndex =
                (m_selectedIndex + static_cast<int>(m_options.size()) - 1) % static_cast<int>(m_options.size());
            return true;
        }

        if (input.GetKeyboard().WasKeyPressed(Key::D) || input.GetKeyboard().WasKeyPressed(Key::Right))
        {
            m_selectedIndex = (m_selectedIndex + 1) % static_cast<int>(m_options.size());
            return true;
        }

        return false;
    }

    void SetSelectedIndex(const int index) noexcept
    {
        if (m_options.empty())
        {
            m_selectedIndex = 0;
            return;
        }

        m_selectedIndex = std::clamp(index, 0, static_cast<int>(m_options.size()) - 1);
    }

    [[nodiscard]] int SelectedIndex() const noexcept
    {
        return m_selectedIndex;
    }

    [[nodiscard]] const std::string& SelectedText() const noexcept
    {
        static const std::string kEmpty;

        if (m_options.empty())
        {
            return kEmpty;
        }

        return m_options[static_cast<std::size_t>(m_selectedIndex)];
    }

    [[nodiscard]] const std::string& Label() const noexcept
    {
        return m_label;
    }

    [[nodiscard]] float X() const noexcept
    {
        return m_x;
    }

    [[nodiscard]] float Y() const noexcept
    {
        return m_y;
    }

    [[nodiscard]] float Width() const noexcept
    {
        return m_width;
    }

    [[nodiscard]] float Height() const noexcept
    {
        return m_height;
    }

    [[nodiscard]] float Right() const noexcept
    {
        return m_x + m_width;
    }

private:
    float m_x{0.0f};
    float m_y{0.0f};
    float m_width{0.0f};
    float m_height{0.0f};

    std::string m_label{};
    std::vector<std::string> m_options{};
    int m_selectedIndex{0};
};



#endif //PINGPONG_SWITCHER_H