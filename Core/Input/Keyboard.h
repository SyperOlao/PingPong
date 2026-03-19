//
// Created by SyperOlao on 17.03.2026.
//

#ifndef PINGPONG_KEYBOARD_H
#define PINGPONG_KEYBOARD_H

#include <Windows.h>

#include <array>
#include <cstdint>

enum class Key : std::uint16_t {
    None = 0,
    W = 'W',
    S = 'S',
    D = 'D',
    A = 'A',
    Left = VK_LEFT,
    Right = VK_RIGHT,
    Up = VK_UP,
    Down = VK_DOWN,
    Escape = VK_ESCAPE,
    Enter = VK_RETURN,
    F1 = VK_F1
};

class Keyboard final {
public:
    void Update();

    [[nodiscard]] bool IsKeyDown(Key key) const noexcept;

    [[nodiscard]] bool WasKeyPressed(Key key) const noexcept;

    [[nodiscard]] bool WasKeyReleased(Key key) const noexcept;

    [[nodiscard]] bool IsVirtualKeyDown(USHORT virtualKey) const noexcept;

    [[nodiscard]] bool WasVirtualKeyPressed(USHORT virtualKey) const noexcept;

    [[nodiscard]] bool WasVirtualKeyReleased(USHORT virtualKey) const noexcept;

private:
    static constexpr std::size_t KeyCount = 256;

    std::array<bool, KeyCount> m_currentState{};
    std::array<bool, KeyCount> m_previousState{};
};


#endif //PINGPONG_KEYBOARD_H
