//
// Created by SyperOlao on 17.03.2026.
//

#include "Window.h"
#include <stdexcept>
#include "../Input/RawInputHandler.h"

Window::~Window() {
    if (m_hWnd != nullptr) {
        DestroyWindow(m_hWnd);
        m_hWnd = nullptr;
    }
}

void Window::Initialize(HINSTANCE__ *const hInstance, const int width, const int height, const std::wstring_view title) {
    m_hInstance = hInstance;
    m_width = width;
    m_height = height;

    RegisterWindowClass();
    CreateAppWindow(title);
}


void Window::RegisterWindowClass() const {
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = &Window::WindowProc;
    wc.hInstance = m_hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;
    wc.lpszClassName = m_className.c_str();

    if (RegisterClassExW(&wc) == 0) {
        throw std::runtime_error("RegisterClassExW failed");
    }
}

void Window::CreateAppWindow(const std::wstring_view title) {
    RECT rect{0, 0, m_width, m_height};

    if (!AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE)) {
        throw std::runtime_error("AdjustWindowRect failed");
    }

    m_hWnd = CreateWindowExW(
        0,
        m_className.c_str(),
        std::wstring(title).c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        nullptr,
        nullptr,
        m_hInstance,
        this
    );

    if (m_hWnd == nullptr) {
        throw std::runtime_error("CreateWindowExW failed");
    }

    ShowWindow(m_hWnd, SW_SHOW);
    UpdateWindow(m_hWnd);
}
bool Window::ProcessMessages() {
    MSG msg {};
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            return false;
        }

        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return true;
}

HWND Window::GetHandle() const noexcept
{
    return m_hWnd;
}

int Window::GetWidth() const noexcept
{
    return m_width;
}

int Window::GetHeight() const noexcept
{
    return m_height;
}

LRESULT CALLBACK Window::WindowProc(HWND__ *const hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam)
{
    if (message == WM_NCCREATE)
    {
        const auto* createStruct = reinterpret_cast<CREATESTRUCTW*>(lParam);
        auto* window = static_cast<Window*>(createStruct->lpCreateParams);

        SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
        window->m_hWnd = hWnd;
    }

    auto* window = reinterpret_cast<Window*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
    if (window != nullptr)
    {
        return window->HandleMessage(message, wParam, lParam);
    }

    return DefWindowProcW(hWnd, message, wParam, lParam);
}

LRESULT Window::HandleMessage(const UINT message, const WPARAM wParam, const LPARAM lParam) const
{
    switch (message)
    {
        case WM_INPUT:
            return RawInputHandler::Instance().HandleMessage(message, wParam, lParam);

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProcW(m_hWnd, message, wParam, lParam);
    }
}