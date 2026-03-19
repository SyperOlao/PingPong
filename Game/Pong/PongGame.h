//
// Created by SyperOlao on 17.03.2026.
//

#ifndef PINGPONG_GAME_H
#define PINGPONG_GAME_H

#include <array>
#include <string>

#include "Core/App/IGame.h"
#include "Core/Common/Types.h"
#include "Core/Input/InputSystem.h"
#include "Core/UI/Button.h"
#include "Core/UI/Switcher.h"
#include "Game/Pong/Entities/Ball.h"
#include "Game/Pong/Entities/Paddle.h"
#include "Game/Pong/PongScene.h"
#include "Game/Pong/PongTypes.h"
#include "Game/Pong/Systems/PaddleAI.h"

struct AppContext;

class PongGame final : public IGame
{
public:
    void Initialize(AppContext& context) override;
    void Update(AppContext& context, float deltaTime) override;
    void Render(AppContext& context) override;

private:
    enum class ScreenState : std::uint8_t
    {
        MainMenu = 0,
        Settings,
        Playing,
        GameOver
    };

private:
    void ApplyDifficulty();
    void StartGame(GameMode mode);
    void ResetRound();
    void ResetMatch();

    void UpdateMainMenu(AppContext& context);
    void UpdateSettingsMenu(AppContext& context);
    void UpdateGameplay(AppContext& context, float deltaTime);
    void UpdateGameOver(const AppContext& context);

    static void UpdatePlayerPaddle(const AppContext& context, Paddle& paddle, InputPlayer player, float deltaTime);
    void UpdateBall(float deltaTime);

    static void ClampPaddleToField(Paddle& paddle) noexcept;
    void LaunchBallRandomDirection();
    void ScorePoint(CourtSide outSide);
    void FinishMatch();

    [[nodiscard]] bool IsEndlessModeActive() const noexcept;

    void RenderMainMenu(const AppContext& context) const;
    void RenderSettingsMenu(const AppContext& context) const;
    void RenderGameplay(const AppContext& context) const;
    void RenderGameOver(const AppContext& context) const;

    static void RenderButtons(
        const AppContext& context,
        const std::array<Button, 4>& buttons,
        int selectedIndex,
        const char* title
    );

private:
    ScreenState m_screenState{ScreenState::MainMenu};
    GameMode m_gameMode{GameMode::TwoPlayers};
    Difficulty m_difficulty{Difficulty::Medium};
    MatchRule m_matchRule{MatchRule::FirstTo10};

    std::array<Button, 4> m_mainMenuButtons{};
    Switcher m_difficultySwitcher{};
    Switcher m_matchRuleSwitcher{};
    Button m_settingsBackButton{};

    int m_selectedMainMenuIndex{0};
    int m_selectedSettingsIndex{0};

    Paddle m_leftPaddle{};
    Paddle m_rightPaddle{};
    Ball m_ball{};

    PongScene m_scene{};
    PaddleAI m_paddleAI{};

    ScoreType m_leftScore{0};
    ScoreType m_rightScore{0};
    float m_roundResetTimer{0.0f};

    float m_fpsAccumulator{0.0f};
    int m_fpsFrames{0};
    int m_displayFps{0};

    std::string m_resultText{};
};


#endif //PINGPONG_GAME_H
