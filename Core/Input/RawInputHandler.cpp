//
// Created by SyperOlao on 17.03.2026.
//

#include "RawInputHandler.h"
#include <cstddef>
#include <stdexcept>
#include <vector>

RawInputHandler& RawInputHandler::Instance()
{
    static RawInputHandler instance;
    return instance;
}

void RawInputHandler::Initialize(HWND targetWindow)
{
    if (targetWindow == nullptr)
    {
        throw std::invalid_argument("RawInputHandler::Initialize received null window handle");
    }

    m_targetWindow = targetWindow;
    RegisterDevices(targetWindow);
    m_isInitialized = true;
}

LRESULT RawInputHandler::HandleMessage(const UINT message, const WPARAM, const LPARAM lParam)
{
    if (message != WM_INPUT)
    {
        return 1;
    }

    ProcessRawInput(reinterpret_cast<HRAWINPUT>(lParam));
    return 0;
}

void RawInputHandler::ClearFrameDeltas() noexcept
{
    m_mouseDeltaX = 0;
    m_mouseDeltaY = 0;
}

void RawInputHandler::ClearAll() noexcept
{
    m_currentState.fill(false);
    m_mouseDeltaX = 0;
    m_mouseDeltaY = 0;
    m_leftMouseDown = false;
    m_rightMouseDown = false;
}

bool RawInputHandler::IsKeyDown(const USHORT virtualKey) const noexcept
{
    if (virtualKey >= m_currentState.size())
    {
        return false;
    }

    return m_currentState[virtualKey];
}

LONG RawInputHandler::GetMouseDeltaX() const noexcept
{
    return m_mouseDeltaX;
}

LONG RawInputHandler::GetMouseDeltaY() const noexcept
{
    return m_mouseDeltaY;
}

bool RawInputHandler::IsLeftMouseDown() const noexcept
{
    return m_leftMouseDown;
}

bool RawInputHandler::IsRightMouseDown() const noexcept
{
    return m_rightMouseDown;
}

void RawInputHandler::RegisterDevices(HWND__ *const targetWindow)
{
    RAWINPUTDEVICE devices[2]{};

    devices[0].usUsagePage = 0x01;
    devices[0].usUsage = 0x06;
    devices[0].dwFlags = RIDEV_INPUTSINK;
    devices[0].hwndTarget = targetWindow;

    devices[1].usUsagePage = 0x01;
    devices[1].usUsage = 0x02;
    devices[1].dwFlags = RIDEV_INPUTSINK;
    devices[1].hwndTarget = targetWindow;

    if (!RegisterRawInputDevices(devices, 2, sizeof(RAWINPUTDEVICE)))
    {
        throw std::runtime_error("RegisterRawInputDevices failed for keyboard/mouse");
    }
}

void RawInputHandler::ProcessRawInput(HRAWINPUT__ *const rawInputHandle)
{
    if (!m_isInitialized)
    {
        return;
    }

    UINT dataSize = 0;
    if (GetRawInputData(rawInputHandle, RID_INPUT, nullptr, &dataSize, sizeof(RAWINPUTHEADER)) != 0)
    {
        return;
    }

    std::vector<std::byte> buffer(dataSize);
    if (GetRawInputData(rawInputHandle, RID_INPUT, buffer.data(), &dataSize, sizeof(RAWINPUTHEADER)) != dataSize)
    {
        return;
    }

    const auto* rawInput = reinterpret_cast<const RAWINPUT*>(buffer.data());

    if (rawInput->header.dwType == RIM_TYPEKEYBOARD)
    {
        const RAWKEYBOARD& keyboardData = rawInput->data.keyboard;
        const USHORT virtualKey = NormalizeVirtualKey(keyboardData);

        if (virtualKey < m_currentState.size())
        {
            const bool isBreak = (keyboardData.Flags & RI_KEY_BREAK) != 0;
            m_currentState[virtualKey] = !isBreak;
        }

        return;
    }

    if (rawInput->header.dwType == RIM_TYPEMOUSE)
    {
        const RAWMOUSE& mouse = rawInput->data.mouse;

        m_mouseDeltaX += mouse.lLastX;
        m_mouseDeltaY += mouse.lLastY;

        if (mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN)  m_leftMouseDown = true;
        if (mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP)    m_leftMouseDown = false;
        if (mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN) m_rightMouseDown = true;
        if (mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP)   m_rightMouseDown = false;

        return;
    }
}

USHORT RawInputHandler::NormalizeVirtualKey(const RAWKEYBOARD& keyboardData) noexcept
{
    USHORT virtualKey = keyboardData.VKey;
    if (virtualKey == 255)
    {
        return 255;
    }

    const UINT scanCode = keyboardData.MakeCode;
    const bool isExtended = (keyboardData.Flags & RI_KEY_E0) != 0;

    switch (virtualKey)
    {
        case VK_SHIFT:
            virtualKey = static_cast<USHORT>(MapVirtualKeyW(scanCode, MAPVK_VSC_TO_VK_EX));
            break;
        case VK_CONTROL:
            virtualKey = static_cast<USHORT>(isExtended ? VK_RCONTROL : VK_LCONTROL);
            break;
        case VK_MENU:
            virtualKey = static_cast<USHORT>(isExtended ? VK_RMENU : VK_LMENU);
            break;
        default:
            break;
    }

    return virtualKey;
}