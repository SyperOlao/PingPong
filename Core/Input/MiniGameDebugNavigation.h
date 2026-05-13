#pragma once

#include "InputSystem.h"

[[nodiscard]] inline bool WasDebugReturnToMainMenuPressed(const InputSystem &input)
{
    return input.GetKeyboard().WasKeyPressed(Key::P);
}
