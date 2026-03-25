//
// Created by SyperOlao on 19.03.2026.
//
#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif


#include <algorithm>
#include <string>
#include <string_view>

#include "Core/App/AppContext.h"
#include "Game/Pong/Common/Constants.h"
#include "Core/Graphics/Color.h"
#include "Core/Graphics/Rendering/ShapeRenderer2D.h"
#include "Core/Input/InputSystem.h"
#include "Core/UI/BitmapFont.h"
#include "Game/Pong/Entities/Ball.h"
#include "Game/Pong/Entities/Paddle.h"
#include "Game/Pong/PongScene.h"
#include "Game/Pong/Systems/PongRules.h"
#include "Core/Audio/AudioSystem.h"
#include "PongUI.h"

namespace {
    constexpr Color kWhite{1.0f, 1.0f, 1.0f, 1.0f};
    constexpr Color kMuted{0.65f, 0.68f, 0.78f, 1.0f};
    constexpr Color kAccent{0.82f, 0.84f, 0.93f, 1.0f};

    constexpr float kMenuButtonWidth = 420.0f;
    constexpr float kMenuButtonHeight = 64.0f;
    constexpr float kMenuButtonGap = 18.0f;

    constexpr float kMenuTitleY = 60.0f;
    constexpr float kMenuSubtitleY = 145.0f;

    struct MenuItemLayout {
        float X{0.0f};
        float Y{0.0f};
        float Width{0.0f};
        float Height{0.0f};
    };

    bool WasMenuUpPressed(const InputSystem &input) {
        return input.GetKeyboard().WasKeyPressed(Key::W)
               || input.GetKeyboard().WasKeyPressed(Key::Up);
    }

    bool WasMenuDownPressed(const InputSystem &input) {
        return input.GetKeyboard().WasKeyPressed(Key::S)
               || input.GetKeyboard().WasKeyPressed(Key::Down);
    }

    MenuItemLayout BuildCenteredMenuItemLayout(const int index, const int itemCount = 4) {
        const float totalHeight =
            static_cast<float>(itemCount) * kMenuButtonHeight
            + static_cast<float>(itemCount - 1) * kMenuButtonGap;

        constexpr float reservedTop = 230.0f;
        constexpr float reservedBottom = 110.0f;

        constexpr float availableHeight =
            static_cast<float>(Constants::WindowHeight) - reservedTop - reservedBottom;

        const float startY =
            reservedTop + std::max(0.0f, (availableHeight - totalHeight) * 0.5f);

        const float x =
            (static_cast<float>(Constants::WindowWidth) - kMenuButtonWidth) * 0.5f;

        const float y =
            startY + static_cast<float>(index) * (kMenuButtonHeight + kMenuButtonGap);

        return MenuItemLayout{x, y, kMenuButtonWidth, kMenuButtonHeight};
    }

    RectF BuildCenteredButtonRect(const int index, const int itemCount = 4) {
        const auto layout = BuildCenteredMenuItemLayout(index, itemCount);
        return RectF{layout.X, layout.Y, layout.Width, layout.Height};
    }

    void DrawCenteredText(
        ShapeRenderer2D &renderer,
        const std::string_view text,
        const float y,
        const Color &color,
        const float scale
    ) {
        const std::string value{text};
        const float width = BitmapFont::MeasureTextWidth(value, scale);
        const float x = (static_cast<float>(Constants::WindowWidth) - width) * 0.5f;
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

    void RenderSwitcherRow(
        const ShapeRenderer2D &renderer,
        const Switcher &switcher,
        const bool selected
    ) {
        const float textY = switcher.Y() + 18.0f;

        BitmapFont::DrawString(
            renderer,
            switcher.X() + 18.0f,
            textY,
            switcher.Label(),
            selected ? kAccent : kMuted,
            0.72f
        );

        const std::string value = std::string{"< "} + switcher.SelectedText() + " >";

        DrawRightAlignedText(
            renderer,
            value,
            switcher.Right() - 18.0f,
            textY,
            selected ? kWhite : kMuted,
            0.72f
        );
    }
}

void PongUI::Initialize() {
    m_mainMenuButtons[0] = Button{BuildCenteredButtonRect(0, 4), "START VS PLAYER", true};
    m_mainMenuButtons[1] = Button{BuildCenteredButtonRect(1, 4), "START VS BOT", true};
    m_mainMenuButtons[2] = Button{BuildCenteredButtonRect(2, 4), "SETTINGS", true};
    m_mainMenuButtons[3] = Button{BuildCenteredButtonRect(3, 4), "EXIT", true};

    {
        const auto layout = BuildCenteredMenuItemLayout(0, 3);
        m_difficultySwitcher = Switcher{
            layout.X,
            layout.Y,
            layout.Width,
            layout.Height,
            "DIFFICULTY",
            {"EASY", "MEDIUM", "HARD"}
        };
    }

    {
        const auto [X, Y, Width, Height] = BuildCenteredMenuItemLayout(1, 3);
        m_matchRuleSwitcher = Switcher{
            X,
            Y,
            Width,
            Height,
            "MODE",
            {"FIRST TO 10", "ENDLESS"}
        };
    }

    m_settingsBackButton = Button{BuildCenteredButtonRect(2, 3), "BACK", true};

    SyncSettings(m_difficulty, m_matchRule);
}

PongUI::Action PongUI::Update(AppContext &context) {
    switch (m_screenState) {
        case ScreenState::MainMenu:
            return UpdateMainMenu(context);

        case ScreenState::Settings:
            return UpdateSettingsMenu(context);

        case ScreenState::Playing:
            return UpdatePlaying(context);

        case ScreenState::GameOver:
            return UpdateGameOver(context);

        default:
            return Action::None;
    }
}

void PongUI::Render(
    const AppContext &context,
    const PongScene &scene,
    const Paddle &leftPaddle,
    const Paddle &rightPaddle,
    const Ball &ball,
    const ScoreType leftScore,
    const ScoreType rightScore,
    const GameMode gameMode
) const {
    switch (m_screenState) {
        case ScreenState::MainMenu:
            RenderMainMenu(context);
            break;

        case ScreenState::Settings:
            RenderSettingsMenu(context);
            break;

        case ScreenState::Playing:
            RenderPlaying(
                context,
                scene,
                leftPaddle,
                rightPaddle,
                ball,
                leftScore,
                rightScore,
                gameMode
            );
            break;

        case ScreenState::GameOver:
            RenderGameOver(
                context,
                scene,
                leftPaddle,
                rightPaddle,
                ball,
                leftScore,
                rightScore,
                gameMode
            );
            break;

        default:
            break;
    }
}

void PongUI::SetScreen(const ScreenState screen) noexcept {
    m_screenState = screen;
}

PongUI::ScreenState PongUI::GetScreen() const noexcept {
    return m_screenState;
}

void PongUI::SyncSettings(const Difficulty difficulty, const MatchRule matchRule) {
    m_difficulty = difficulty;
    m_matchRule = matchRule;

    m_difficultySwitcher.SetSelectedIndex(static_cast<int>(m_difficulty));
    m_matchRuleSwitcher.SetSelectedIndex(static_cast<int>(m_matchRule));
}

Difficulty PongUI::SelectedDifficulty() const noexcept {
    return m_difficulty;
}

MatchRule PongUI::SelectedMatchRule() const noexcept {
    return m_matchRule;
}

void PongUI::SetGameOverText(const std::string &text) {
    m_resultText = text;
}

void PongUI::ClearGameOverText() {
    m_resultText.clear();
}

void PongUI::SetDisplayedFps(const int fps) noexcept {
    m_displayFps = fps;
}

PongUI::Action PongUI::UpdateMainMenu(AppContext &context) {
    auto &input = *context.Input;

    const int previousIndex = m_selectedMainMenuIndex;

    if (WasMenuUpPressed(input)) {
        m_selectedMainMenuIndex = (m_selectedMainMenuIndex + 3) % 4;
    } else if (WasMenuDownPressed(input)) {
        m_selectedMainMenuIndex = (m_selectedMainMenuIndex + 1) % 4;
    }

    if (previousIndex != m_selectedMainMenuIndex) {
        context.Audio->PlayOneShot("ui_move", 0.45f);
    }

    if (!m_mainMenuButtons[m_selectedMainMenuIndex].HandleKeyboard(input, true)) {
        return Action::None;
    }

    context.Audio->PlayOneShot("ui_accept", 0.65f);

    switch (m_selectedMainMenuIndex) {
        case 0:
            m_screenState = ScreenState::Playing;
            return Action::StartVsPlayer;

        case 1:
            m_screenState = ScreenState::Playing;
            return Action::StartVsBot;

        case 2:
            m_difficultySwitcher.SetSelectedIndex(static_cast<int>(m_difficulty));
            m_matchRuleSwitcher.SetSelectedIndex(static_cast<int>(m_matchRule));
            m_selectedSettingsIndex = 0;
            m_screenState = ScreenState::Settings;
            return Action::None;

        case 3:
            return Action::ExitRequested;

        default:
            return Action::None;
    }
}

PongUI::Action PongUI::UpdateSettingsMenu(AppContext &context) {
    auto &input = *context.Input;

    if (input.GetKeyboard().WasKeyPressed(Key::Escape)) {
        context.Audio->PlayOneShot("ui_accept", 0.65f);
        m_screenState = ScreenState::MainMenu;
        return Action::None;
    }

    const int previousIndex = m_selectedSettingsIndex;

    if (WasMenuUpPressed(input)) {
        m_selectedSettingsIndex = (m_selectedSettingsIndex + 2) % 3;
    } else if (WasMenuDownPressed(input)) {
        m_selectedSettingsIndex = (m_selectedSettingsIndex + 1) % 3;
    }

    if (previousIndex != m_selectedSettingsIndex) {
        context.Audio->PlayOneShot("ui_move", 0.45f);
    }

    if (m_selectedSettingsIndex == 0) {
        if (m_difficultySwitcher.HandleKeyboard(input)) {
            context.Audio->PlayOneShot("ui_move", 0.5f);
            m_difficulty = static_cast<Difficulty>(m_difficultySwitcher.SelectedIndex());
            return Action::DifficultyChanged;
        }

        return Action::None;
    }

    if (m_selectedSettingsIndex == 1) {
        if (m_matchRuleSwitcher.HandleKeyboard(input)) {
            context.Audio->PlayOneShot("ui_move", 0.5f);
            m_matchRule = static_cast<MatchRule>(m_matchRuleSwitcher.SelectedIndex());
            return Action::MatchRuleChanged;
        }

        return Action::None;
    }

    if (m_settingsBackButton.HandleKeyboard(input, true)) {
        context.Audio->PlayOneShot("ui_accept", 0.65f);
        m_screenState = ScreenState::MainMenu;
    }

    return Action::None;
}

PongUI::Action PongUI::UpdatePlaying(AppContext &context) {
    auto &input = *context.Input;

    if (input.GetKeyboard().WasKeyPressed(Key::Escape)) {
        context.Audio->PlayOneShot("ui_accept", 0.65f);
        m_screenState = ScreenState::MainMenu;
        return Action::None;
    }

    if (input.GetKeyboard().WasKeyPressed(Key::Enter)) {
        context.Audio->PlayOneShot("ui_accept", 0.65f);
        return Action::ResetMatch;
    }

    return Action::None;
}

PongUI::Action PongUI::UpdateGameOver(AppContext &context) {
    auto &input = *context.Input;

    if (input.GetKeyboard().WasKeyPressed(Key::Escape)) {
        context.Audio->PlayOneShot("ui_accept", 0.65f);
        m_screenState = ScreenState::MainMenu;
        return Action::None;
    }

    if (input.GetKeyboard().WasKeyPressed(Key::Enter)) {
        context.Audio->PlayOneShot("ui_accept", 0.65f);
        return Action::RestartGame;
    }

    return Action::None;
}

void PongUI::RenderMainMenu(const AppContext &context) const {
    auto &renderer = *context.Shape2D;

    RenderButtons(context, m_mainMenuButtons, m_selectedMainMenuIndex, "PONG");

    DrawCenteredText(
        renderer,
        PongRules::BuildDifficultyText(m_difficulty),
        kMenuSubtitleY,
        kMuted,
        0.95f
    );

    BitmapFont::DrawString(
        renderer,
        26.0f,
        static_cast<float>(Constants::WindowHeight) - 44.0f,
        "W/S OR ARROWS  NAVIGATE",
        kMuted,
        0.62f
    );

    DrawRightAlignedText(
        renderer,
        "ENTER  SELECT",
        static_cast<float>(Constants::WindowWidth) - 26.0f,
        static_cast<float>(Constants::WindowHeight) - 44.0f,
        kMuted,
        0.62f
    );
}

void PongUI::RenderSettingsMenu(const AppContext &context) const {
    auto &renderer = *context.Shape2D;

    DrawCenteredText(renderer, "SETTINGS", kMenuTitleY, kWhite, 1.8f);

    DrawCenteredText(
        renderer,
        "VS PLAYER ALWAYS USES FIRST TO 10",
        kMenuSubtitleY,
        kMuted,
        0.72f
    );

    RenderSwitcherRow(renderer, m_difficultySwitcher, m_selectedSettingsIndex == 0);
    RenderSwitcherRow(renderer, m_matchRuleSwitcher, m_selectedSettingsIndex == 1);

    ButtonStyle style{};
    style.TextScale = 0.85f;

    m_settingsBackButton.Draw(
        renderer,
        *context.Font,
        style,
        m_selectedSettingsIndex == 2
    );

    BitmapFont::DrawString(
        renderer,
        26.0f,
        static_cast<float>(Constants::WindowHeight) - 44.0f,
        "W/S  ROW   A/D OR LEFT/RIGHT  CHANGE",
        kMuted,
        0.58f
    );

    DrawRightAlignedText(
        renderer,
        "ESC OR ENTER ON BACK",
        static_cast<float>(Constants::WindowWidth) - 26.0f,
        static_cast<float>(Constants::WindowHeight) - 44.0f,
        kMuted,
        0.58f
    );
}

void PongUI::RenderPlaying(
    const AppContext &context,
    const PongScene &scene,
    const Paddle &leftPaddle,
    const Paddle &rightPaddle,
    const Ball &ball,
    const ScoreType leftScore,
    const ScoreType rightScore,
    const GameMode gameMode
) const {
    scene.RenderGameplay(
        context,
        leftPaddle,
        rightPaddle,
        ball,
        leftScore,
        rightScore,
        m_displayFps,
        gameMode,
        m_difficulty
    );
}

void PongUI::RenderGameOver(
    const AppContext &context,
    const PongScene &scene,
    const Paddle &leftPaddle,
    const Paddle &rightPaddle,
    const Ball &ball,
    const ScoreType leftScore,
    const ScoreType rightScore,
    const GameMode gameMode
) const {
    auto &renderer = *context.Shape2D;

    RenderPlaying(
        context,
        scene,
        leftPaddle,
        rightPaddle,
        ball,
        leftScore,
        rightScore,
        gameMode
    );

    constexpr float centerY = static_cast<float>(Constants::WindowHeight) * 0.5f;

    DrawCenteredText(
        renderer,
        m_resultText,
        centerY - 70.0f,
        kWhite,
        2.35f
    );

    DrawCenteredText(
        renderer,
        "ENTER  RESTART",
        centerY + 20.0f,
        kAccent,
        0.88f
    );

    DrawCenteredText(
        renderer,
        "ESC  MAIN MENU",
        centerY + 58.0f,
        kMuted,
        0.76f
    );
}

void PongUI::RenderButtons(
    const AppContext &context,
    const std::array<Button, 4> &buttons,
    const int selectedIndex,
    const char *title
) {
    auto &renderer = *context.Shape2D;

    DrawCenteredText(renderer, title, kMenuTitleY, kWhite, 1.8f);

    ButtonStyle style{};
    style.TextScale = 0.85f;

    for (int index = 0; index < static_cast<int>(buttons.size()); ++index) {
        buttons[static_cast<std::size_t>(index)].Draw(
            renderer,
            *context.Font,
            style,
            index == selectedIndex
        );
    }
}