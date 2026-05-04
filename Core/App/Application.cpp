//
// Created by SyperOlao on 17.03.2026.
//

#include "Application.h"

#include "AppContext.h"
#include "FatalErrorReport.h"
#include "Core/Graphics/GraphicsDevice.h"
#include "Core/Input/RawInputHandler.h"

#include <stdexcept>
#include <string>
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
    m_renderContext.BuildDefaultForwardRenderPipeline();
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
    try {
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
    catch (const std::exception &exception) {
        const std::string message = std::string("Runtime error:\n") + exception.what();
        FatalErrorReport::Report(message);
        try {
            m_game->Shutdown(m_context);
        }
        catch (...) {
        }
        ShutdownEngineServices();
        return -1;
    }
    catch (...) {
        FatalErrorReport::Report("Unknown runtime error (non-std::exception).");
        try {
            m_game->Shutdown(m_context);
        }
        catch (...) {
        }
        ShutdownEngineServices();
        return -1;
    }
}

void Application::Update(const float deltaTime) {
    m_window.SyncClientDimensions();
    const int clientWidth = m_window.GetWidth();
    const int clientHeight = m_window.GetHeight();
    if (clientWidth > 0 && clientHeight > 0)
    {
        if (clientWidth != m_graphics.GetWidth() || clientHeight != m_graphics.GetHeight())
        {
            m_graphics.Resize(clientWidth, clientHeight);
            m_renderContext.ResizeDeferredResources();
        }
    }

    m_renderContext.GetDebugDraw().Clear();
    m_input.Update();
    m_audio.Update();
    m_game->Update(m_context, deltaTime);
    UpdateGlobalRenderModeButton();
}

void Application::Render() {
    m_renderContext.BeginRenderFrame();
    m_renderContext.GetFrameRenderer().BeginFrame(m_desc.ClearColor);
    m_renderContext.ExecuteFramePipeline(
        nullptr,
        nullptr,
        [&]() {
            m_game->Render(m_context);
        },
        m_timer.GetDeltaTime()
    );
    RenderGlobalRenderModeButton();
    m_renderContext.GetFrameRenderer().EndFrame(m_desc.VSync);
}

void Application::UpdateGlobalRenderModeButton() {
    if (m_context.Platform.MainWindow == nullptr) {
        return;
    }

    const int windowWidth = m_context.Platform.MainWindow->GetWidth();
    const int windowHeight = m_context.Platform.MainWindow->GetHeight();
    if (windowWidth <= 0 || windowHeight <= 0) {
        return;
    }

    m_globalRenderModeButton.Bounds = RectF{
        20.0f,
        static_cast<float>(windowHeight) - 68.0f,
        360.0f,
        48.0f
    };

    const RenderMode renderMode = m_context.GetRenderMode();
    if (renderMode == RenderMode::Deferred) {
        m_globalRenderModeButton.Label = "RENDER: DEFERRED";
    } else {
        m_globalRenderModeButton.Label = "RENDER: FORWARD";
    }

    POINT cursorPoint{};
    GetCursorPos(&cursorPoint);
    ScreenToClient(m_context.Platform.MainWindow->GetHandle(), &cursorPoint);

    const bool isLeftMouseDown = RawInputHandler::Instance().IsLeftMouseDown();
    const bool wasLeftPressed = isLeftMouseDown && !m_previousLeftMouseDown;
    m_previousLeftMouseDown = isLeftMouseDown;

    if (m_globalRenderModeButton.HandleMouseClick(
        static_cast<float>(cursorPoint.x),
        static_cast<float>(cursorPoint.y),
        wasLeftPressed
    )) {
        const RenderMode PreviousRenderMode = m_context.GetRenderMode();
        m_context.ToggleRenderMode();
        const RenderMode CurrentRenderMode = m_context.GetRenderMode();
        if (CurrentRenderMode != PreviousRenderMode) {
            m_game->OnRenderModeChanged(m_context, CurrentRenderMode);
        }
    }
}

void Application::RenderGlobalRenderModeButton() const {
    if (m_context.Ui.Font == nullptr || m_context.Platform.MainWindow == nullptr) {
        return;
    }

    POINT cursorPoint{};
    GetCursorPos(&cursorPoint);
    ScreenToClient(m_context.Platform.MainWindow->GetHandle(), &cursorPoint);
    const bool isHovered = m_globalRenderModeButton.IsHovered(
        static_cast<float>(cursorPoint.x),
        static_cast<float>(cursorPoint.y)
    );

    ButtonStyle buttonStyle{};
    buttonStyle.TextScale = 0.72f;
    m_globalRenderModeButton.Draw(
        m_context.GetShapeRenderer2D(),
        *m_context.Ui.Font,
        buttonStyle,
        isHovered
    );
}
