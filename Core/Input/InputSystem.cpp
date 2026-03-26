//
// Created by SyperOlao on 17.03.2026.
//

#include "InputSystem.h"
#include "RawInputHandler.h"


void InputSystem::Initialize(HWND__ *const targetWindow)
{
    RawInputHandler::Instance().Initialize(targetWindow);
}

void InputSystem::Update()
{
    m_keyboard.Update();
}

void InputSystem::Clear()
{
    RawInputHandler::Instance().ClearAll();
    m_keyboard.Update();
}

const Keyboard& InputSystem::GetKeyboard() const noexcept
{
    return m_keyboard;
}

Keyboard& InputSystem::GetKeyboard() noexcept
{
    return m_keyboard;
}

LONG InputSystem::GetMouseDeltaX() const noexcept
{
    return RawInputHandler::Instance().GetMouseDeltaX();
}

LONG InputSystem::GetMouseDeltaY() const noexcept
{
    return RawInputHandler::Instance().GetMouseDeltaY();
}

bool InputSystem::IsRightMouseDown() const noexcept
{
    return RawInputHandler::Instance().IsRightMouseDown();
}

float InputSystem::GetVerticalAxis(const InputPlayer player) const noexcept
{
    const float up = IsActionDown(player, InputAction::MoveUp) ? 1.0f : 0.0f;
    const float down = IsActionDown(player, InputAction::MoveDown) ? 1.0f : 0.0f;

    return down - up;
}

bool InputSystem::IsActionDown(const InputPlayer player, const InputAction action) const noexcept
{
    const Key key = ResolveKey(player, action);
    if (key == Key::None)
    {
        return false;
    }

    return m_keyboard.IsKeyDown(key);
}

bool InputSystem::WasActionPressed(const InputPlayer player, const InputAction action) const noexcept
{
    const Key key = ResolveKey(player, action);
    if (key == Key::None)
    {
        return false;
    }

    return m_keyboard.WasKeyPressed(key);
}

void InputSystem::SetPlayerBindings(const InputPlayer player, const PlayerBindings& bindings) noexcept
{
    m_bindings[static_cast<std::size_t>(player)] = bindings;
}

PlayerBindings InputSystem::GetPlayerBindings(const InputPlayer player) const noexcept
{
    return m_bindings[static_cast<std::size_t>(player)];
}

Key InputSystem::ResolveKey(const InputPlayer player, const InputAction action) const noexcept
{
    const auto&[MoveUp, MoveDown] = m_bindings[static_cast<std::size_t>(player)];

    switch (action)
    {
        case InputAction::MoveUp:
            return MoveUp;
        case InputAction::MoveDown:
            return MoveDown;
        case InputAction::Pause:
            return Key::Escape;
        case InputAction::Restart:
            return Key::Enter;
        case InputAction::ToggleMode:
            return Key::F1;
        default:
            return Key::None;
    }
}