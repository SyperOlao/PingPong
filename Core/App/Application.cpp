//
// Created by SyperOlao on 17.03.2026.
//

#include "Application.h"

Application::Application(HINSTANCE hInstance)
    : m_hInstance(hInstance)
{
    Initialize();
}

void Application::Initialize()
{
    constexpr int windowWidth = 1280;
    constexpr int windowHeight = 720;

    m_window.Initialize(m_hInstance, windowWidth, windowHeight, L"Pong DX11");
    m_renderer.Initialize(m_window.GetHandle(), m_window.GetWidth(), m_window.GetHeight());
    m_game.Initialize(m_window.GetWidth(), m_window.GetHeight());
    m_timer.Reset();
}

int Application::Run()
{
    while (m_isRunning)
    {
        if (!Window::ProcessMessages())
        {
            m_isRunning = false;
            break;
        }

        m_timer.Tick();
        Update();
        Render();
    }

    return 0;
}

void Application::Update()
{
    m_game.Update(m_timer.GetDeltaTime());
}

void Application::Render() const {
    constexpr Color clearColor {0.05f, 0.05f, 0.08f, 1.0f};

    m_renderer.BeginFrame(clearColor);
    m_game.Render(m_renderer);
    m_renderer.EndFrame();
}
