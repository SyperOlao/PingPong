#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "Game/MainMenu/MainMenuGame.h"

#include "Core/App/AppContext.h"
#include "Core/App/IGameHost.h"
#include "Core/Audio/AudioSystem.h"
#include "Core/Platform/Window.h"
#include "Core/Graphics/Color.h"
#include "Core/Graphics/Rendering/ShapeRenderer2D.h"
#include "Core/Input/InputSystem.h"
#include "Core/Input/Keyboard.h"
#include "Core/Input/RawInputHandler.h"
#include "Core/UI/BitmapFont.h"
#include "Game/Katamari/KatamariGame.h"
#include "Game/LightingTest/LightingTestGame.h"
#include "Game/Pong/PongGame.h"
#include "Game/SolarSystem/SolarSystemGame.h"

#include <algorithm>
#include <string>
#include <string_view>

#include <Windows.h>

namespace {
constexpr Color kWhite{1.0f, 1.0f, 1.0f, 1.0f};
constexpr Color kMuted{0.65f, 0.68f, 0.78f, 1.0f};

constexpr float kMenuButtonWidth = 420.0f;
constexpr float kMenuButtonHeight = 64.0f;
constexpr float kMenuButtonGap = 18.0f;
constexpr float kMenuTitleY = 60.0f;
constexpr float kMenuSubtitleY = 145.0f;
constexpr int kMenuItemCount = 5;

struct MenuItemLayout {
    float X{0.0f};
    float Y{0.0f};
    float Width{0.0f};
    float Height{0.0f};
};

MenuItemLayout BuildCenteredMenuItemLayout(const float windowWidth, const float windowHeight, const int index) {
    const float totalHeight =
        static_cast<float>(kMenuItemCount) * kMenuButtonHeight
        + static_cast<float>(kMenuItemCount - 1) * kMenuButtonGap;

    constexpr float reservedTop = 230.0f;
    constexpr float reservedBottom = 110.0f;

    const float availableHeight = windowHeight - reservedTop - reservedBottom;

    const float startY =
        reservedTop + std::max(0.0f, (availableHeight - totalHeight) * 0.5f);

    const float x = (windowWidth - kMenuButtonWidth) * 0.5f;

    const float y =
        startY + static_cast<float>(index) * (kMenuButtonHeight + kMenuButtonGap);

    return MenuItemLayout{x, y, kMenuButtonWidth, kMenuButtonHeight};
}

RectF BuildCenteredButtonRect(const float windowWidth, const float windowHeight, const int index) {
    const auto layout = BuildCenteredMenuItemLayout(windowWidth, windowHeight, index);
    return RectF{layout.X, layout.Y, layout.Width, layout.Height};
}

void DrawCenteredText(
    ShapeRenderer2D &renderer,
    const std::string_view text,
    const float windowWidth,
    const float y,
    const Color &color,
    const float scale
) {
    const std::string value{text};
    const float width = BitmapFont::MeasureTextWidth(value, scale);
    const float x = (windowWidth - width) * 0.5f;
    BitmapFont::DrawString(renderer, x, y, value, color, scale);
}

void DrawRightAlignedText(
    const ShapeRenderer2D &renderer,
    const std::string_view text,
    const float rightX,
    const float y,
    const Color &color,
    const float scale
) {
    const std::string value{text};
    const float width = BitmapFont::MeasureTextWidth(value, scale);
    BitmapFont::DrawString(renderer, rightX - width, y, value, color, scale);
}

bool WasMenuUpPressed(const InputSystem &input) {
    return input.GetKeyboard().WasKeyPressed(Key::W)
           || input.GetKeyboard().WasKeyPressed(Key::Up);
}

bool WasMenuDownPressed(const InputSystem &input) {
    return input.GetKeyboard().WasKeyPressed(Key::S)
           || input.GetKeyboard().WasKeyPressed(Key::Down);
}
}

void MainMenuGame::Initialize(AppContext &context) {
    m_menuButtons[0].Label = "PONG";
    m_menuButtons[1].Label = "SOLAR SYSTEM";
    m_menuButtons[2].Label = "KATAMARI";
    m_menuButtons[3].Label = "LIGHTING TEST";
    m_menuButtons[4].Label = "EXIT";

    for (Button &button : m_menuButtons) {
        button.Enabled = true;
    }

    m_selectedMenuIndex = 0;
    // Input that triggered a game switch must not leak into this menu.
    // Reading the real button state prevents the switching click from
    // appearing as a fresh press on the very first Update() after a switch.
    m_previousLeftMouseDown = RawInputHandler::Instance().IsLeftMouseDown();

    if (context.Audio.System != nullptr) {
        context.Audio.System->Load("ui_move", "Game/Pong/Assets/UI_MOVE.wav");
        context.Audio.System->Load("ui_accept", "Game/Pong/Assets/UI_ACCEPT.wav");
    }

    SyncButtonLayout(context);
}

void MainMenuGame::Shutdown(AppContext &) {
}

bool MainMenuGame::TryGetPreferredClearColor(Color &clearColor) const noexcept {
    clearColor = Color(0.06f, 0.07f, 0.12f, 1.0f);
    return true;
}

void MainMenuGame::SyncButtonLayout(const AppContext &context) {
    if (context.Platform.MainWindow == nullptr) {
        return;
    }

    const float windowWidth = static_cast<float>(context.Platform.MainWindow->GetWidth());
    const float windowHeight = static_cast<float>(context.Platform.MainWindow->GetHeight());
    if (windowWidth <= 0.0f || windowHeight <= 0.0f) {
        return;
    }

    for (int index = 0; index < kMenuItemCount; ++index) {
        m_menuButtons[static_cast<std::size_t>(index)].Bounds =
            BuildCenteredButtonRect(windowWidth, windowHeight, index);
    }
}

void MainMenuGame::TryActivateSelection(AppContext &context, const int itemIndex) {
    if (context.GameHost == nullptr) {
        return;
    }

    if (context.Audio.System != nullptr) {
        context.Audio.System->PlayOneShot("ui_accept", 0.65f);
    }

    switch (itemIndex) {
        case 0:
            context.GameHost->RequestSwitchGame(std::make_unique<PongGame>());
            break;

        case 1:
            context.GameHost->RequestSwitchGame(std::make_unique<SolarSystemGame>());
            break;

        case 2:
            context.GameHost->RequestSwitchGame(std::make_unique<KatamariGame>());
            break;

        case 3:
            context.GameHost->RequestSwitchGame(std::make_unique<LightingTestGame>());
            break;

        case 4:
            context.GameHost->RequestQuitApplication();
            break;

        default:
            break;
    }
}

void MainMenuGame::Update(AppContext &context, float) {
    SyncButtonLayout(context);

    if (context.Platform.MainWindow == nullptr || context.Input.System == nullptr) {
        return;
    }

    auto &input = *context.Input.System;

    const int previousIndex = m_selectedMenuIndex;

    if (WasMenuUpPressed(input)) {
        m_selectedMenuIndex = (m_selectedMenuIndex + kMenuItemCount - 1) % kMenuItemCount;
    } else if (WasMenuDownPressed(input)) {
        m_selectedMenuIndex = (m_selectedMenuIndex + 1) % kMenuItemCount;
    }

    if (previousIndex != m_selectedMenuIndex && context.Audio.System != nullptr) {
        context.Audio.System->PlayOneShot("ui_move", 0.45f);
    }

    POINT cursorPoint{};
    GetCursorPos(&cursorPoint);
    ScreenToClient(context.Platform.MainWindow->GetHandle(), &cursorPoint);

    const float mouseX = static_cast<float>(cursorPoint.x);
    const float mouseY = static_cast<float>(cursorPoint.y);

    const bool isLeftMouseDown = RawInputHandler::Instance().IsLeftMouseDown();
    const bool wasLeftPressed = isLeftMouseDown && !m_previousLeftMouseDown;
    m_previousLeftMouseDown = isLeftMouseDown;

    if (wasLeftPressed) {
        for (int index = 0; index < kMenuItemCount; ++index) {
            if (m_menuButtons[static_cast<std::size_t>(index)].HandleMouseClick(mouseX, mouseY, true)) {
                TryActivateSelection(context, index);
                return;
            }
        }
    }

    if (m_menuButtons[static_cast<std::size_t>(m_selectedMenuIndex)].HandleKeyboard(input, true)) {
        TryActivateSelection(context, m_selectedMenuIndex);
    }
}

void MainMenuGame::Render(AppContext &context) {
    if (context.Ui.Font == nullptr || context.Platform.MainWindow == nullptr) {
        return;
    }

    POINT cursorPoint{};
    GetCursorPos(&cursorPoint);
    ScreenToClient(context.Platform.MainWindow->GetHandle(), &cursorPoint);

    const float mouseX = static_cast<float>(cursorPoint.x);
    const float mouseY = static_cast<float>(cursorPoint.y);

    auto &renderer = context.GetShapeRenderer2D();
    const float windowWidth = static_cast<float>(context.Platform.MainWindow->GetWidth());
    const float windowHeight = static_cast<float>(context.Platform.MainWindow->GetHeight());

    DrawCenteredText(renderer, "MINI ENGINE", windowWidth, kMenuTitleY, kWhite, 1.8f);
    DrawCenteredText(renderer, "CHOOSE A DEMO", windowWidth, kMenuSubtitleY, kMuted, 0.95f);

    ButtonStyle style{};
    style.TextScale = 0.85f;

    for (int index = 0; index < kMenuItemCount; ++index) {
        const bool isHighlighted =
            index == m_selectedMenuIndex
            || m_menuButtons[static_cast<std::size_t>(index)].IsHovered(mouseX, mouseY);

        m_menuButtons[static_cast<std::size_t>(index)].Draw(
            renderer,
            *context.Ui.Font,
            style,
            isHighlighted
        );
    }

    BitmapFont::DrawString(
        renderer,
        26.0f,
        windowHeight - 44.0f,
        "W/S OR ARROWS  NAVIGATE",
        kMuted,
        0.62f
    );

    DrawRightAlignedText(
        renderer,
        "ENTER OR CLICK  SELECT",
        windowWidth - 26.0f,
        windowHeight - 44.0f,
        kMuted,
        0.62f
    );
}
