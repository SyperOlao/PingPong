//
// Created by SyperOlao on 17.03.2026.
//

#ifndef PINGPONG_INPUTSYSTEM_H
#define PINGPONG_INPUTSYSTEM_H
#include <Windows.h>

#include <array>
#include <cstdint>

#include "Keyboard.h"

enum class InputPlayer : std::size_t
{
    Player1 = 0,
    Player2 = 1
};

enum class InputAction : std::uint8_t
{
    MoveUp = 0,
    MoveDown,
    Pause,
    Restart,
    ToggleMode
};

struct PlayerBindings final
{
    Key MoveUp{Key::W};
    Key MoveDown{Key::S};
};

class InputSystem final
{
public:
    static void Initialize(HWND targetWindow);
    void Update();
    void Clear();

    [[nodiscard]] const Keyboard& GetKeyboard() const noexcept;
    [[nodiscard]] Keyboard& GetKeyboard() noexcept;
    [[nodiscard]] LONG GetMouseDeltaX() const noexcept;
    [[nodiscard]] LONG GetMouseDeltaY() const noexcept;
    [[nodiscard]] bool IsRightMouseDown() const noexcept;

    /**
     * Возвращает ось Y:
     * -1 = вверх
     *  0 = покой
     * +1 = вниз
     */
    [[nodiscard]] float GetVerticalAxis(InputPlayer player) const noexcept;

    [[nodiscard]] bool IsActionDown(InputPlayer player, InputAction action) const noexcept;
    [[nodiscard]] bool WasActionPressed(InputPlayer player, InputAction action) const noexcept;

    void SetPlayerBindings(InputPlayer player, const PlayerBindings& bindings) noexcept;
    [[nodiscard]] PlayerBindings GetPlayerBindings(InputPlayer player) const noexcept;

private:
    [[nodiscard]] Key ResolveKey(InputPlayer player, InputAction action) const noexcept;

private:
    Keyboard m_keyboard{};
    std::array<PlayerBindings, 2> m_bindings
    {
        PlayerBindings{Key::W, Key::S},
        PlayerBindings{Key::Up, Key::Down}
    };
};

#endif //PINGPONG_INPUTSYSTEM_H