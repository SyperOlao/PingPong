//
// Created by SyperOlao on 17.03.2026.
//

#include "Application.h"

#include "AppContext.h"
#include "Core/Graphics/GraphicsDevice.h"
#include <stdexcept>
#include <utility>

Application::Application(
    HINSTANCE__ *const hInstance,
    std::unique_ptr<IGame> game,
    ApplicationDesc desc
)
    : m_hInstance(hInstance)
      , m_desc(std::move(desc))
      , m_game(std::move(game)) {
    if (m_hInstance == nullptr) {
        throw std::invalid_argument("Application requires a valid HINSTANCE.");
    }

    if (!m_game) {
        throw std::invalid_argument("Application requires a valid IGame instance.");
    }

    Initialize();
}

void Application::Initialize() {
    if (m_desc.ClientWidth <= 0 || m_desc.ClientHeight <= 0) {
        throw std::invalid_argument("ApplicationDesc must contain positive client dimensions.");
    }

    m_window.Initialize(
        m_hInstance,
        m_desc.ClientWidth,
        m_desc.ClientHeight,
        m_desc.Title
    );

    InputSystem::Initialize(m_window.GetHandle());

    m_graphics.Initialize(
        m_window.GetHandle(),
        m_window.GetWidth(),
        m_window.GetHeight()
    );

    m_audio.Initialize();

    m_renderContext.Initialize(m_graphics);
    m_assetCache.Initialize(m_graphics);

    m_context.MainWindow = &m_window;
    m_context.Input = &m_input;
    m_context.Graphics = &m_graphics;
    m_context.Shape2D = &m_renderContext.GetShapeRenderer2D();
    m_context.Font = &m_bitmapFont;
    m_context.Audio = &m_audio;
    m_context.Assets = &m_assetCache;

    if (!m_context.IsValid()) {
        throw std::runtime_error("Application failed to build a valid AppContext.");
    }

    m_game->Initialize(m_context);
    m_timer.Reset();
}

int Application::Run() {
    while (m_isRunning) {
        if (!Window::ProcessMessages()) {
            m_isRunning = false;
            break;
        }

        m_timer.Tick();

        Update(m_timer.GetDeltaTime());
        Render();
    }

    m_game->Shutdown(m_context);
    m_audio.Shutdown();
    return 0;
}

void Application::Update(const float deltaTime) {
    m_input.Update();
    m_audio.Update();
    m_game->Update(m_context, deltaTime);
}

void Application::Render() {
    m_graphics.BeginFrame(m_desc.ClearColor);
    m_game->Render(m_context);
    m_graphics.EndFrame(m_desc.VSync);
}
