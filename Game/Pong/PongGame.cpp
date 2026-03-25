#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "Game/Pong/PongGame.h"

#include <algorithm>
#include <cmath>
#include <string_view>

#include "Core/App/AppContext.h"
#include "Game/Pong/Common/Constants.h"
#include "Core/Graphics/Color.h"
#include "Core/Input/InputSystem.h"
#include "Core/Math/MathHelpers.h"
#include "Game/Pong/Systems/PongCollisionSystem.h"
#include "Game/Pong/Systems/PongRules.h"
#include "Core/Audio/AudioSystem.h"

namespace {
    constexpr ScoreType kWinningScore = 10;
}

void PongGame::Initialize(AppContext &context) {
    m_leftPaddle.SetDimensions(Constants::PaddleWidth, Constants::PaddleHeight);
    m_rightPaddle.SetDimensions(Constants::PaddleWidth, Constants::PaddleHeight);
    m_ball.SetSize(Constants::BallSize);

    m_leftPaddle.ColorTint = {1.0f, 1.0f, 1.0f, 1.0f};
    m_rightPaddle.ColorTint = {1.0f, 1.0f, 1.0f, 1.0f};
    m_ball.ColorTint = {1.0f, 1.0f, 1.0f, 1.0f};

    m_ui.Initialize();
    m_ui.SyncSettings(m_difficulty, m_matchRule);
    m_ui.SetScreen(PongUI::ScreenState::MainMenu);

    context.Audio.System->Load("ui_move", "Game/Pong/Assets/UI_MOVE.wav");
    context.Audio.System->Load("ui_accept", "Game/Pong/Assets/UI_ACCEPT.wav");
    context.Audio.System->Load("paddle_hit", "Game/Pong/Assets/PING.wav");
    context.Audio.System->Load("point", "Game/Pong/Assets/POINT.wav");
    context.Audio.System->Load("wall_hit", "Game/Pong/Assets/BALL_WALL.wav");
    context.Audio.System->Load("score", "Game/Pong/Assets/POINT.wav");
    context.Audio.System->Load("music_game", "Game/Pong/Assets/MUSIC.wav");

    context.Audio.System->StartLoop("music_game", 0.35f);

    m_scene.Initialize();

    ResetMatch();
}

void PongGame::Update(AppContext &context, const float deltaTime) {
    m_fpsAccumulator += deltaTime;
    ++m_fpsFrames;

    if (m_fpsAccumulator >= 0.25f) {
        m_displayFps = static_cast<int>(std::round(static_cast<float>(m_fpsFrames) / m_fpsAccumulator));
        m_fpsAccumulator = 0.0f;
        m_fpsFrames = 0;
    }

    m_ui.SetDisplayedFps(m_displayFps);

    switch (m_ui.GetScreen()) {
        case PongUI::ScreenState::MainMenu:
        case PongUI::ScreenState::Settings:
        case PongUI::ScreenState::GameOver:
            HandleUiAction(m_ui.Update(context));
            return;

        case PongUI::ScreenState::Playing: {
            const auto action = m_ui.Update(context);
            HandleUiAction(action);

            if (m_ui.GetScreen() != PongUI::ScreenState::Playing || action == PongUI::Action::ResetMatch) {
                return;
            }

            UpdateGameplay(context, deltaTime);
            return;
        }

        default:
            return;
    }
}

void PongGame::Render(AppContext &context) {
    m_ui.Render(
        context,
        m_scene,
        m_leftPaddle,
        m_rightPaddle,
        m_ball,
        m_leftScore,
        m_rightScore,
        m_gameMode
    );
}

void PongGame::HandleUiAction(const PongUI::Action action) {
    switch (action) {
        case PongUI::Action::StartVsPlayer:
            StartGame(GameMode::TwoPlayers);
            break;

        case PongUI::Action::StartVsBot:
            StartGame(GameMode::VersusAI);
            break;

        case PongUI::Action::ExitRequested:
            PostQuitMessage(0);
            break;

        case PongUI::Action::DifficultyChanged:
            m_difficulty = m_ui.SelectedDifficulty();
            ApplyDifficulty();
            break;

        case PongUI::Action::MatchRuleChanged:
            m_matchRule = m_ui.SelectedMatchRule();
            break;

        case PongUI::Action::ResetMatch:
            ResetMatch();
            break;

        case PongUI::Action::RestartGame:
            StartGame(m_gameMode);
            break;

        case PongUI::Action::None:
        default:
            break;
    }
}

void PongGame::ApplyDifficulty() {
    const auto &tuning = PongRules::GetDifficultyTuning(m_difficulty);

    m_leftPaddle.SetDimensions(Constants::PaddleWidth, tuning.PaddleHeight);
    m_rightPaddle.SetDimensions(Constants::PaddleWidth, tuning.PaddleHeight);

    const float centerY =
            (PongRules::GetPlayableTop() + PongRules::GetPlayableBottom()) * 0.5f;

    m_leftPaddle.Transform.SetPosition(
        Constants::PaddleMarginX,
        centerY - m_leftPaddle.Height() * 0.5f
    );

    m_rightPaddle.Transform.SetPosition(
        static_cast<float>(Constants::WindowWidth) - Constants::PaddleMarginX - m_rightPaddle.Width(),
        centerY - m_rightPaddle.Height() * 0.5f
    );

    ResetRound();
}

void PongGame::StartGame(const GameMode mode) {
    m_gameMode = mode;
    m_ui.SetScreen(PongUI::ScreenState::Playing);
    m_ui.ClearGameOverText();
    ResetMatch();
}

void PongGame::ResetRound() {
    const float centerX =
            static_cast<float>(Constants::WindowWidth) * 0.5f - m_ball.Size() * 0.5f;

    const float centerY =
            (PongRules::GetPlayableTop() + PongRules::GetPlayableBottom()) * 0.5f - m_ball.Size() * 0.5f;

    m_ball.Transform.SetPosition(centerX, centerY);
    m_ball.Movement.Stop();

    m_paddleAI.Reset();
    m_roundResetTimer = Constants::BallResetDelaySeconds;
}

void PongGame::ResetMatch() {
    m_leftScore = 0;
    m_rightScore = 0;
    ApplyDifficulty();
}

void PongGame::UpdateGameplay(const AppContext &context, const float deltaTime) {
    UpdatePlayerPaddle(context, m_leftPaddle, InputPlayer::Player1, deltaTime);

    if (m_gameMode == GameMode::TwoPlayers) {
        UpdatePlayerPaddle(context, m_rightPaddle, InputPlayer::Player2, deltaTime);
    } else {
        m_paddleAI.Update(
            m_rightPaddle,
            m_ball,
            PongRules::GetDifficultyTuning(m_difficulty),
            deltaTime
        );

        ClampPaddleToField(m_rightPaddle);
    }

    if (m_roundResetTimer > 0.0f) {
        m_roundResetTimer = std::max(0.0f, m_roundResetTimer - deltaTime);

        if (m_roundResetTimer == 0.0f) {
            LaunchBallRandomDirection();
        }

        return;
    }

    UpdateBall(deltaTime, context);

    if (PongCollisionSystem::HandlePaddleCollision(m_ball, m_leftPaddle, m_rightPaddle)) {
        context.Audio.System->PlayOneShot("paddle_hit", 0.75f);
    }

    const CourtSide outOfBounds = PongCollisionSystem::CheckScoring(m_ball);
    if (outOfBounds != CourtSide::None) {
        context.Audio.System->PlayOneShot("point", 0.85f);
        ScorePoint(outOfBounds);
    }
}

void PongGame::UpdatePlayerPaddle(
    const AppContext &context,
    Paddle &paddle,
    const InputPlayer player,
    const float deltaTime
) {
    const float axis = context.Input.System->GetVerticalAxis(player);
    paddle.Transform.Position.y += axis * Constants::PaddleSpeed * deltaTime;
    ClampPaddleToField(paddle);
}

void PongGame::UpdateBall(const float deltaTime, const AppContext &context) {
    m_ball.Movement.Update(m_ball.Transform, deltaTime);

    if (PongCollisionSystem::HandleWallCollision(
        m_ball,
        PongRules::GetPlayableTop(),
        PongRules::GetPlayableBottom()
    )) {
        context.Audio.System->PlayOneShot("wall_hit", 0.45f);
    }
}

void PongGame::ClampPaddleToField(Paddle &paddle) noexcept {
    const float minY = PongRules::GetPlayableTop();
    const float maxY = PongRules::GetPlayableBottom() - paddle.Height();
    paddle.Transform.Position.y = MathHelpers::Clamp(paddle.Transform.Position.y, minY, maxY);
}

void PongGame::LaunchBallRandomDirection() {
    const float directionX = MathHelpers::RandomFloat(0.0f, 1.0f) < 0.5f ? -1.0f : 1.0f;
    const float directionY = MathHelpers::RandomFloat(-0.75f, 0.75f);

    DirectX::SimpleMath::Vector2 direction{directionX, directionY};
    direction = MathHelpers::SafeNormalize(direction, DirectX::SimpleMath::Vector2{directionX, 0.0f});

    m_ball.Movement.Velocity = direction * PongRules::GetDifficultyTuning(m_difficulty).BallStartSpeed;
}

void PongGame::ScorePoint(const CourtSide outSide) {
    if (outSide == CourtSide::Left) {
        ++m_rightScore;
    } else if (outSide == CourtSide::Right) {
        ++m_leftScore;
    }

    if (!IsEndlessModeActive() && (m_leftScore >= kWinningScore || m_rightScore >= kWinningScore)) {
        FinishMatch();
        return;
    }

    ResetRound();
}

void PongGame::FinishMatch() {
    m_ball.Movement.Stop();
    m_roundResetTimer = 0.0f;

    if (m_gameMode == GameMode::VersusAI) {
        m_ui.SetGameOverText((m_leftScore > m_rightScore) ? "WIN" : "LOSE");
    } else {
        m_ui.SetGameOverText((m_leftScore > m_rightScore) ? "PLAYER 1 WIN" : "PLAYER 2 WIN");
    }

    m_ui.SetScreen(PongUI::ScreenState::GameOver);
}

bool PongGame::IsEndlessModeActive() const noexcept {
    return m_gameMode == GameMode::VersusAI && m_matchRule == MatchRule::Endless;
}
