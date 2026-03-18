//
// Created by SyperOlao on 17.03.2026.
//

#ifndef PINGPONG_WINDOW_H
#define PINGPONG_WINDOW_H
#include <Windows.h>
#include <string>
#include <string_view>
class Window {
public:
    Window() = default;
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;

    void Initialize(HINSTANCE hInstance, int width, int height, std::wstring_view title);

    [[nodiscard]] static bool ProcessMessages();
    [[nodiscard]] HWND GetHandle() const noexcept;
    [[nodiscard]] int GetWidth() const noexcept;
    [[nodiscard]] int GetHeight() const noexcept;

private:
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam) const;

    void RegisterWindowClass() const;
    void CreateAppWindow(std::wstring_view title);

private:
    HINSTANCE m_hInstance {nullptr};
    HWND m_hWnd {nullptr};

    int m_width {1280};
    int m_height {720};

    std::wstring m_className {L"PongEngineWindowClass"};
};


#endif //PINGPONG_WINDOW_H