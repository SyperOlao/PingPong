//
// Created by SyperOlao on 17.03.2026.
//

#ifndef PINGPONG_GAME_H
#define PINGPONG_GAME_H

#include <array>
#include <string>

#include "Core/Math/DXIncludes.h"

#include "Core/App/IGame.h"
#include "Core/Common/Types.h"
#include "Core/Graphics/Color.h"
#include "Core/Input/InputSystem.h"
#include "Core/Physics/AABB.h"
#include "Game/Pong/Components/Collider2D.h"
#include "Game/Pong/Components/MovementComponent.h"
#include "Game/Pong/Components/Transform2D.h"
#include "Core/UI/Button.h"

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
        Playing
    };

    enum class Difficulty : std::uint8_t
    {
        Easy = 0,
        Medium,
        Hard
    };

    struct PaddleEntity final
    {
        Transform2D Transform{};
        MovementComponent Movement{};
        Collider2D Collider{};
        Color ColorTint{1.0f, 1.0f, 1.0f, 1.0f};

        [[nodiscard]] float Width() const noexcept;
        [[nodiscard]] float Height() const noexcept;
        [[nodiscard]] AABB GetAABB() const noexcept;
    };

    struct BallEntity final
    {
        Transform2D Transform{};
        MovementComponent Movement{};
        Collider2D Collider{};
        Color ColorTint{1.0f, 1.0f, 1.0f, 1.0f};

        [[nodiscard]] float Size() const noexcept;
        [[nodiscard]] AABB GetAABB() const noexcept;
    };

    struct DifficultyTuning final
    {
        const char* Label{nullptr};
        float BallStartSpeed{0.0f};
        float PaddleHeight{0.0f};
        float AIPaddleSpeed{0.0f};
        float AIMistakeChancePerSecond{0.0f};
        float AIMistakeDurationMin{0.0f};
        float AIMistakeDurationMax{0.0f};
        float AITrackingError{0.0f};
        float AIMistakeSpeedMultiplier{1.0f};
    };

    struct AIState final
    {
        float MistakeTimer{0.0f};
        float CurrentTrackingOffset{0.0f};
    };

private:
    void ApplyDifficulty();
    void StartGame(GameMode mode);
    void ResetRound();
    void ResetMatch();

    void UpdateMainMenu(AppContext& context);
    void UpdateSettingsMenu(AppContext& context);
    void UpdateGameplay(AppContext& context, float deltaTime);

    static void UpdatePlayerPaddle(const AppContext& context, PaddleEntity& paddle, InputPlayer player, float deltaTime);
    void UpdateAIPaddle(float deltaTime);
    void UpdateBall(float deltaTime);
    void HandleBallCollisions();

    static void ClampPaddleToField(PaddleEntity& paddle) noexcept;
    void LaunchBallRandomDirection();
    void ScorePoint(CourtSide outSide);

    void RenderMainMenu(const AppContext& context) const;
    void RenderSettingsMenu(const AppContext& context) const;
    void RenderGameplay(const AppContext& context) const;
    void RenderPlayField(const AppContext& context) const;
    void RenderHUD(const AppContext& context) const;

    static void RenderButtons(const AppContext& context, const std::array<Button, 4>& buttons, int selectedIndex, const char* title);

    [[nodiscard]] const DifficultyTuning& GetDifficultyTuning() const noexcept;
    [[nodiscard]] static float GetPlayableTop() noexcept;
    [[nodiscard]] static float GetPlayableBottom() noexcept;
    [[nodiscard]] float GetBallSpeed() const noexcept;
    [[nodiscard]] std::string BuildScoreText() const;
    [[nodiscard]] std::string BuildModeText() const;
    [[nodiscard]] std::string BuildDifficultyText() const;
    [[nodiscard]] std::string BuildFpsText() const;

private:
    static constexpr float FieldMarginTop = 80.0f;
    static constexpr float FieldMarginBottom = 40.0f;

    ScreenState m_screenState{ScreenState::MainMenu};
    GameMode m_gameMode{GameMode::TwoPlayers};
    Difficulty m_difficulty{Difficulty::Medium};

    std::array<Button, 4> m_mainMenuButtons{};
    std::array<Button, 4> m_settingsButtons{};
    int m_selectedMainMenuIndex{0};
    int m_selectedSettingsIndex{0};

    PaddleEntity m_leftPaddle{};
    PaddleEntity m_rightPaddle{};
    BallEntity m_ball{};

    AIState m_aiState{};

    ScoreType m_leftScore{0};
    ScoreType m_rightScore{0};
    float m_roundResetTimer{0.0f};

    float m_fpsAccumulator{0.0f};
    int m_fpsFrames{0};
    int m_displayFps{0};
};

#endif //PINGPONG_GAME_H
