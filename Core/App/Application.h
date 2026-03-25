//
// Created by SyperOlao on 17.03.2026.
//

#ifndef PINGPONG_APPLICATION_H
#define PINGPONG_APPLICATION_H
#include <Windows.h>

#include <memory>
#include <string>

#include "ApplicationDefaults.h"
#include "../Platform/Window.h"
#include "../Graphics/Color.h"
#include "../Graphics/GraphicsDevice.h"
#include "../Graphics/Rendering/RenderContext.h"
#include "../Input/InputSystem.h"
#include "../Assets/AssetCache.h"
#include "AppContext.h"
#include "IGame.h"
#include "Timer.h"
#include "Core/UI/BitmapFont.h"
#include "Core/Audio/AudioSystem.h"

struct ApplicationDesc final {
    std::wstring Title{L"DX11 Application"};
    int ClientWidth{ApplicationDefaults::DefaultClientWidth};
    int ClientHeight{ApplicationDefaults::DefaultClientHeight};
    bool VSync{true};
    Color ClearColor{0.05f, 0.05f, 0.08f, 1.0f};
};

class Window;
class InputSystem;
class GraphicsDevice;
class ShapeRenderer2D;
class BitmapFont;
class IGame;

class Application final {
public:
    Application(HINSTANCE hInstance, std::unique_ptr<IGame> game, ApplicationDesc desc = {});

    ~Application() = default;

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    Application(Application&&) = delete;
    Application& operator=(Application&&) = delete;

    int Run();

private:
    void Initialize();
    void Update(float deltaTime);
    void Render();

private:
    HINSTANCE m_hInstance{nullptr};
    ApplicationDesc m_desc{};
    std::unique_ptr<IGame> m_game;

    Window m_window;
    InputSystem m_input;
    GraphicsDevice m_graphics;
    RenderContext m_renderContext{};
    BitmapFont m_bitmapFont{};
    Timer m_timer{};
    AppContext m_context{};
    AudioSystem m_audio{};
    AssetCache m_assetCache{};
    bool m_isRunning{true};
};


#endif //PINGPONG_APPLICATION_H
