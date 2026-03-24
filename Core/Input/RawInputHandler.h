//
// Created by SyperOlao on 17.03.2026.
//

#ifndef PINGPONG_RAWINPUTHANDLER_H
#define PINGPONG_RAWINPUTHANDLER_H
#include <Windows.h>
#include <array>

class RawInputHandler final {
public:
    static RawInputHandler& Instance();

    RawInputHandler(const RawInputHandler&) = delete;
    RawInputHandler& operator=(const RawInputHandler&) = delete;
    RawInputHandler(RawInputHandler&&) = delete;
    RawInputHandler& operator=(RawInputHandler&&) = delete;

    void Initialize(HWND targetWindow);
    [[nodiscard]] LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);

    void ClearFrameDeltas() noexcept;
    void ClearAll() noexcept;

    [[nodiscard]] bool IsKeyDown(USHORT virtualKey) const noexcept;

    [[nodiscard]] LONG GetMouseDeltaX() const noexcept;
    [[nodiscard]] LONG GetMouseDeltaY() const noexcept;

    [[nodiscard]] bool IsLeftMouseDown() const noexcept;
    [[nodiscard]] bool IsRightMouseDown() const noexcept;

private:
    RawInputHandler() = default;

    static void RegisterDevices(HWND targetWindow);
    void ProcessRawInput(HRAWINPUT rawInputHandle);

    static USHORT NormalizeVirtualKey(const RAWKEYBOARD& keyboardData) noexcept;

private:
    HWND m_targetWindow{nullptr};
    bool m_isInitialized{false};

    std::array<bool, 256> m_currentState{};

    LONG m_mouseDeltaX{0};
    LONG m_mouseDeltaY{0};

    bool m_leftMouseDown{false};
    bool m_rightMouseDown{false};
};


#endif //PINGPONG_RAWINPUTHANDLER_H
