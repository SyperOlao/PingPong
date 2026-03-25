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

    m_context.Platform.MainWindow = &m_window;
    m_context.Input.System = &m_input;
    m_context.Graphics.Device = &m_graphics;
    m_context.Graphics.Render = &m_renderContext;
    m_context.Graphics.Frame = &m_renderContext.GetFrameRenderer();
    m_context.Ui.Font = &m_bitmapFont;
    m_context.Audio.System = &m_audio;
    m_context.Assets.Cache = &m_assetCache;

    if (!m_context.IsValid()) {
        throw std::runtime_error("Application failed to build a valid AppContext.");
    }

    m_game->Initialize(m_context);
    m_timer.Reset();
}

void Application::ShutdownEngineServices() {
    m_audio.Shutdown();
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
    ShutdownEngineServices();
    return 0;
}

void Application::Update(const float deltaTime) {
    m_renderContext.GetDebugDraw().Clear();
    m_input.Update();
    m_audio.Update();
    m_game->Update(m_context, deltaTime);
}

void Application::Render() {
    m_renderContext.GetFrameRenderer().BeginFrame(m_desc.ClearColor);
    m_game->Render(m_context);
    m_renderContext.GetFrameRenderer().EndFrame(m_desc.VSync);
}
