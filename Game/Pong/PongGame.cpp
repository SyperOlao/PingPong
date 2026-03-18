//
// Created by SyperOlao on 17.03.2026.
//
#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "PongGame.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <string>

#include "Core/App/AppContext.h"
#include "Core/Common/Constants.h"
#include "Core/Graphics2D/ShapeRenderer2D.h"
#include "Core/Input/InputSystem.h"
#include "Core/Math/MathHelpers.h"
#include "Core/Physics/CollisionSystem.h"
#include "Core/UI/BitmapFont.h"

#include "../../Core/Graphics/Color.h"
#include "../../Core/App/AppContext.h"
#include "../../Core/Graphics2D/ShapeRenderer2D.h"
#include "Core/Math/DXIncludes.h"

struct AppContext;

namespace {
    constexpr Color kWhite{1.0f, 1.0f, 1.0f, 1.0f};
    constexpr Color kMuted{0.65f, 0.68f, 0.78f, 1.0f};
    constexpr Color kAccent{0.82f, 0.84f, 0.93f, 1.0f};

    constexpr float kMenuButtonWidth = 420.0f;
    constexpr float kMenuButtonHeight = 64.0f;
    constexpr float kMenuButtonGap = 18.0f;

    bool WasMenuUpPressed(const InputSystem &input) {
        return input.GetKeyboard().WasKeyPressed(Key::W)
               || input.GetKeyboard().WasKeyPressed(Key::Up);
    }

    bool WasMenuDownPressed(const InputSystem &input) {
        return input.GetKeyboard().WasKeyPressed(Key::S)
               || input.GetKeyboard().WasKeyPressed(Key::Down);
    }

    RectF BuildCenteredButtonRect(const int index) {
        const float totalHeight = 4.0f * kMenuButtonHeight + 3.0f * kMenuButtonGap;
        const float startY = (static_cast<float>(Constants::WindowHeight) - totalHeight) * 0.5f + 70.0f;
        const float x = (static_cast<float>(Constants::WindowWidth) - kMenuButtonWidth) * 0.5f;
        const float y = startY + static_cast<float>(index) * (kMenuButtonHeight + kMenuButtonGap);

        return RectF{x, y, kMenuButtonWidth, kMenuButtonHeight};
    }
}

float PongGame::PaddleEntity::Width() const noexcept {
    return Collider.HalfSize.x * 2.0f;
}

float PongGame::PaddleEntity::Height() const noexcept {
    return Collider.HalfSize.y * 2.0f;
}

AABB PongGame::PaddleEntity::GetAABB() const noexcept {
    return Collider.GetWorldAABB(Transform);
}

float PongGame::BallEntity::Size() const noexcept {
    return Collider.HalfSize.x * 2.0f;
}

AABB PongGame::BallEntity::GetAABB() const noexcept {
    return Collider.GetWorldAABB(Transform);
}

void PongGame::Initialize(AppContext &context) {
    (void) context;

    m_mainMenuButtons[0] = Button{BuildCenteredButtonRect(0), "START VS PLAYER", true};
    m_mainMenuButtons[1] = Button{BuildCenteredButtonRect(1), "START VS BOT", true};
    m_mainMenuButtons[2] = Button{BuildCenteredButtonRect(2), "SETTINGS", true};
    m_mainMenuButtons[3] = Button{BuildCenteredButtonRect(3), "EXIT", true};

    m_settingsButtons[0] = Button{BuildCenteredButtonRect(0), "EASY", true};
    m_settingsButtons[1] = Button{BuildCenteredButtonRect(1), "MEDIUM", true};
    m_settingsButtons[2] = Button{BuildCenteredButtonRect(2), "HARD", true};
    m_settingsButtons[3] = Button{BuildCenteredButtonRect(3), "BACK", true};

    m_leftPaddle.Collider.HalfSize = DirectX::SimpleMath::Vector2(Constants::PaddleWidth * 0.5f,
                                                                  Constants::PaddleHeight * 0.5f);
    m_rightPaddle.Collider.HalfSize = DirectX::SimpleMath::Vector2(Constants::PaddleWidth * 0.5f,
                                                                   Constants::PaddleHeight * 0.5f);
    m_ball.Collider.HalfSize = DirectX::SimpleMath::Vector2(Constants::BallSize * 0.5f, Constants::BallSize * 0.5f);

    m_leftPaddle.ColorTint = kWhite;
    m_rightPaddle.ColorTint = kWhite;
    m_ball.ColorTint = kWhite;

    ApplyDifficulty();
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

    switch (m_screenState) {
        case ScreenState::MainMenu:
            UpdateMainMenu(context);
            break;
        case ScreenState::Settings:
            UpdateSettingsMenu(context);
            break;
        case ScreenState::Playing:
            UpdateGameplay(context, deltaTime);
            break;
        default:
            break;
    }
}

void PongGame::Render(AppContext &context) {
    switch (m_screenState) {
        case ScreenState::MainMenu:
            RenderMainMenu(context);
            break;
        case ScreenState::Settings:
            RenderSettingsMenu(context);
            break;
        case ScreenState::Playing:
            RenderGameplay(context);
            break;
        default:
            break;
    }
}

void PongGame::ApplyDifficulty() {
    const auto &tuning = GetDifficultyTuning();

    m_leftPaddle.Collider.HalfSize.y = tuning.PaddleHeight * 0.5f;
    m_rightPaddle.Collider.HalfSize.y = tuning.PaddleHeight * 0.5f;

    const float centerY = (GetPlayableTop() + GetPlayableBottom()) * 0.5f;
    m_leftPaddle.Transform.SetPosition(Constants::PaddleMarginX, centerY - m_leftPaddle.Height() * 0.5f);
    m_rightPaddle.Transform.SetPosition(
        static_cast<float>(Constants::WindowWidth) - Constants::PaddleMarginX - m_rightPaddle.Width(),
        centerY - m_rightPaddle.Height() * 0.5f
    );

    ResetRound();
}

void PongGame::StartGame(const GameMode mode) {
    m_gameMode = mode;
    m_screenState = ScreenState::Playing;
    ResetMatch();
}

void PongGame::ResetRound() {
    const float centerX = static_cast<float>(Constants::WindowWidth) * 0.5f - m_ball.Size() * 0.5f;
    const float centerY = (GetPlayableTop() + GetPlayableBottom()) * 0.5f - m_ball.Size() * 0.5f;

    m_ball.Transform.SetPosition(centerX, centerY);
    m_ball.Movement.Stop();

    m_aiState.MistakeTimer = 0.0f;
    m_aiState.CurrentTrackingOffset = 0.0f;
    m_roundResetTimer = Constants::BallResetDelaySeconds;
}

void PongGame::ResetMatch() {
    m_leftScore = 0;
    m_rightScore = 0;
    ApplyDifficulty();
}

void PongGame::UpdateMainMenu(AppContext &context) {
    auto &input = *context.Input;

    if (WasMenuUpPressed(input)) {
        m_selectedMainMenuIndex = (m_selectedMainMenuIndex + 3) % 4;
    } else if (WasMenuDownPressed(input)) {
        m_selectedMainMenuIndex = (m_selectedMainMenuIndex + 1) % 4;
    }

    if (!m_mainMenuButtons[m_selectedMainMenuIndex].HandleKeyboard(input, true)) {
        return;
    }

    switch (m_selectedMainMenuIndex) {
        case 0:
            StartGame(GameMode::TwoPlayers);
            break;
        case 1:
            StartGame(GameMode::VersusAI);
            break;
        case 2:
            m_screenState = ScreenState::Settings;
            break;
        case 3:
            PostQuitMessage(0);
            break;
        default:
            break;
    }
}

void PongGame::UpdateSettingsMenu(AppContext &context) {
    auto &input = *context.Input;

    if (input.GetKeyboard().WasKeyPressed(Key::Escape)) {
        m_screenState = ScreenState::MainMenu;
        return;
    }

    if (WasMenuUpPressed(input)) {
        m_selectedSettingsIndex = (m_selectedSettingsIndex + 3) % 4;
    } else if (WasMenuDownPressed(input)) {
        m_selectedSettingsIndex = (m_selectedSettingsIndex + 1) % 4;
    }

    if (!m_settingsButtons[m_selectedSettingsIndex].HandleKeyboard(input, true)) {
        return;
    }

    switch (m_selectedSettingsIndex) {
        case 0:
            m_difficulty = Difficulty::Easy;
            ApplyDifficulty();
            break;
        case 1:
            m_difficulty = Difficulty::Medium;
            ApplyDifficulty();
            break;
        case 2:
            m_difficulty = Difficulty::Hard;
            ApplyDifficulty();
            break;
        case 3:
            m_screenState = ScreenState::MainMenu;
            break;
        default:
            break;
    }
}

void PongGame::UpdateGameplay(AppContext &context, const float deltaTime) {
    auto &input = *context.Input;

    if (input.GetKeyboard().WasKeyPressed(Key::Escape)) {
        m_screenState = ScreenState::MainMenu;
        return;
    }

    if (input.GetKeyboard().WasKeyPressed(Key::Enter)) {
        ResetMatch();
        return;
    }

    UpdatePlayerPaddle(context, m_leftPaddle, InputPlayer::Player1, deltaTime);

    if (m_gameMode == GameMode::TwoPlayers) {
        UpdatePlayerPaddle(context, m_rightPaddle, InputPlayer::Player2, deltaTime);
    } else {
        UpdateAIPaddle(deltaTime);
    }

    if (m_roundResetTimer > 0.0f) {
        m_roundResetTimer = std::max(0.0f, m_roundResetTimer - deltaTime);
        if (m_roundResetTimer == 0.0f) {
            LaunchBallRandomDirection();
        }
        return;
    }

    UpdateBall(deltaTime);
    HandleBallCollisions();
}

void PongGame::UpdatePlayerPaddle(
    const AppContext &context,
    PaddleEntity &paddle,
    const InputPlayer player,
    const float deltaTime
) {
    const float axis = context.Input->GetVerticalAxis(player);
    paddle.Transform.Position.y += axis * Constants::PaddleSpeed * deltaTime;
    ClampPaddleToField(paddle);
}

void PongGame::UpdateAIPaddle(const float deltaTime) {
    const auto &tuning = GetDifficultyTuning();

    if (m_aiState.MistakeTimer > 0.0f) {
        m_aiState.MistakeTimer = std::max(0.0f, m_aiState.MistakeTimer - deltaTime);
        if (m_aiState.MistakeTimer == 0.0f) {
            m_aiState.CurrentTrackingOffset = 0.0f;
        }
    } else if (MathHelpers::RandomFloat(0.0f, 1.0f) < tuning.AIMistakeChancePerSecond * deltaTime) {
        m_aiState.MistakeTimer = MathHelpers::RandomFloat(tuning.AIMistakeDurationMin, tuning.AIMistakeDurationMax);
        m_aiState.CurrentTrackingOffset = MathHelpers::RandomFloat(-tuning.AITrackingError, tuning.AITrackingError);
    }

    const float ballCenterY = m_ball.GetAABB().GetCenter().y;
    const float paddleCenterY = m_rightPaddle.GetAABB().GetCenter().y;
    const float targetCenterY = ballCenterY + m_aiState.CurrentTrackingOffset;
    const float deltaY = targetCenterY - paddleCenterY;

    if (std::abs(deltaY) <= Constants::PaddleAIDeadZone) {
        return;
    }

    float moveSpeed = tuning.AIPaddleSpeed;
    if (m_aiState.MistakeTimer > 0.0f) {
        moveSpeed *= tuning.AIMistakeSpeedMultiplier;
    }

    const float direction = deltaY > 0.0f ? 1.0f : -1.0f;
    m_rightPaddle.Transform.Position.y += direction * moveSpeed * deltaTime;
    ClampPaddleToField(m_rightPaddle);
}

void PongGame::UpdateBall(const float deltaTime) {
    m_ball.Movement.Update(m_ball.Transform, deltaTime);

    if (const AABB ballAABB = m_ball.GetAABB(); ballAABB.Min.y <= GetPlayableTop()) {
        m_ball.Transform.Position.y = GetPlayableTop();
        m_ball.Movement.Velocity.y = std::abs(m_ball.Movement.Velocity.y);
    } else if (ballAABB.Max.y >= GetPlayableBottom()) {
        m_ball.Transform.Position.y = GetPlayableBottom() - m_ball.Size();
        m_ball.Movement.Velocity.y = -std::abs(m_ball.Movement.Velocity.y);
    }

    const CourtSide outOfBounds = CollisionSystem::CheckOutOfBounds(
        m_ball.GetAABB(),
        0.0f,
        static_cast<float>(Constants::WindowWidth)
    );

    if (outOfBounds != CourtSide::None) {
        ScorePoint(outOfBounds);
    }
}

void PongGame::HandleBallCollisions() {
    const AABB ballAABB = m_ball.GetAABB();
    const AABB leftPaddleAABB = m_leftPaddle.GetAABB();
    const AABB rightPaddleAABB = m_rightPaddle.GetAABB();

    if (CollisionSystem::CheckCollision(ballAABB, leftPaddleAABB) && m_ball.Movement.Velocity.x < 0.0f) {
        m_ball.Transform.Position.x = leftPaddleAABB.Max.x + 0.5f;

        const float nextSpeed = std::min(
            GetBallSpeed() * Constants::BallSpeedMultiplierOnPaddleHit,
            Constants::BallMaxSpeed
        );

        m_ball.Movement.Velocity = CollisionSystem::BuildPaddleBounceVelocity(
            m_ball.GetAABB(),
            leftPaddleAABB,
            nextSpeed,
            MathHelpers::DegToRad(Constants::MaxBounceAngleDegrees),
            true
        );
        return;
    }

    if (CollisionSystem::CheckCollision(ballAABB, rightPaddleAABB) && m_ball.Movement.Velocity.x > 0.0f) {
        m_ball.Transform.Position.x = rightPaddleAABB.Min.x - m_ball.Size() - 0.5f;

        const float nextSpeed = std::min(
            GetBallSpeed() * Constants::BallSpeedMultiplierOnPaddleHit,
            Constants::BallMaxSpeed
        );

        m_ball.Movement.Velocity = CollisionSystem::BuildPaddleBounceVelocity(
            m_ball.GetAABB(),
            rightPaddleAABB,
            nextSpeed,
            MathHelpers::DegToRad(Constants::MaxBounceAngleDegrees),
            false
        );
    }
}

void PongGame::ClampPaddleToField(PaddleEntity &paddle) noexcept {
    const float minY = GetPlayableTop();
    const float maxY = GetPlayableBottom() - paddle.Height();
    paddle.Transform.Position.y = MathHelpers::Clamp(paddle.Transform.Position.y, minY, maxY);
}

void PongGame::LaunchBallRandomDirection() {
    const float directionX = MathHelpers::RandomFloat(0.0f, 1.0f) < 0.5f ? -1.0f : 1.0f;
    const float directionY = MathHelpers::RandomFloat(-0.75f, 0.75f);

    DirectX::SimpleMath::Vector2 direction{directionX, directionY};
    direction = MathHelpers::SafeNormalize(direction, DirectX::SimpleMath::Vector2{directionX, 0.0f});

    m_ball.Movement.Velocity = direction * GetDifficultyTuning().BallStartSpeed;
}

void PongGame::ScorePoint(const CourtSide outSide) {
    if (outSide == CourtSide::Left) {
        ++m_rightScore;
    } else if (outSide == CourtSide::Right) {
        ++m_leftScore;
    }

    ResetRound();
}

void PongGame::RenderMainMenu(const AppContext &context) const {
    RenderButtons(context, m_mainMenuButtons, m_selectedMainMenuIndex, "PONG");

    BitmapFont::DrawString(
        *context.Shape2D,
        360.0f,
        145.0f,
        BuildDifficultyText(),
        kMuted,
        0.95f
    );
}

void PongGame::RenderSettingsMenu(const AppContext &context) const {
    RenderButtons(context, m_settingsButtons, m_selectedSettingsIndex, "SETTINGS");

    BitmapFont::DrawString(
        *context.Shape2D,
        300.0f,
        145.0f,
        "DIFFICULTY CHANGES BALL SPEED PADDLE SIZE AND BOT ERRORS",
        kMuted,
        0.52f
    );
}

void PongGame::RenderGameplay(const AppContext &context) const {
    RenderPlayField(context);
    RenderHUD(context);
}

void PongGame::RenderPlayField(const AppContext &context) const {
    const auto &renderer = *context.Shape2D;

    const float playableTop = GetPlayableTop();
    const float playableHeight = GetPlayableBottom() - GetPlayableTop();

    renderer.DrawFilledRect(0.0f, playableTop - 4.0f, static_cast<float>(Constants::WindowWidth), 4.0f, kMuted);
    renderer.DrawFilledRect(0.0f, GetPlayableBottom(), static_cast<float>(Constants::WindowWidth), 4.0f, kMuted);

    constexpr float centerX = static_cast<float>(Constants::WindowWidth) * 0.5f - Constants::CenterLineWidth * 0.5f;
    const int segmentCount = static_cast<int>(
        playableHeight / (Constants::CenterLineSegmentHeight + Constants::CenterLineGap));

    for (int index = 0; index < segmentCount; ++index) {
        const float segmentY = playableTop + 16.0f + static_cast<float>(index) * (
                                   Constants::CenterLineSegmentHeight + Constants::CenterLineGap);
        renderer.DrawFilledRect(
            centerX,
            segmentY,
            Constants::CenterLineWidth,
            Constants::CenterLineSegmentHeight,
            Color(0.55f, 0.55f, 0.62f, 1.0f)
        );
    }

    renderer.DrawFilledRect(
        m_leftPaddle.Transform.Position.x,
        m_leftPaddle.Transform.Position.y,
        m_leftPaddle.Width(),
        m_leftPaddle.Height(),
        m_leftPaddle.ColorTint
    );

    renderer.DrawFilledRect(
        m_rightPaddle.Transform.Position.x,
        m_rightPaddle.Transform.Position.y,
        m_rightPaddle.Width(),
        m_rightPaddle.Height(),
        m_rightPaddle.ColorTint
    );

    renderer.DrawFilledRect(
        m_ball.Transform.Position.x,
        m_ball.Transform.Position.y,
        m_ball.Size(),
        m_ball.Size(),
        m_ball.ColorTint
    );
}

void PongGame::RenderHUD(const AppContext &context) const {
    auto &font = *context.Font;
    const auto &renderer = *context.Shape2D;

    const std::string scoreText = BuildScoreText();
    constexpr float scoreScale = 1.3f;
    const float scoreWidth = BitmapFont::MeasureTextWidth(scoreText, scoreScale);
    BitmapFont::DrawString(
        renderer,
        (static_cast<float>(Constants::WindowWidth) - scoreWidth) * 0.5f,
        22.0f,
        scoreText,
        kWhite,
        scoreScale
    );

    BitmapFont::DrawString(renderer, 24.0f, 24.0f, BuildFpsText(), kAccent, 0.9f);
    BitmapFont::DrawString(renderer, 24.0f, 58.0f, BuildModeText(), kMuted, 0.75f);
    BitmapFont::DrawString(renderer, 24.0f, 86.0f, BuildDifficultyText(), kMuted, 0.75f);
    BitmapFont::DrawString(renderer, 24.0f, 114.0f, "ESC MENU  ENTER RESET", kMuted, 0.68f);
}

void PongGame::RenderButtons(
    const AppContext &context,
    const std::array<Button, 4> &buttons,
    const int selectedIndex,
    const char *title
) {
    auto &renderer = *context.Shape2D;
    const auto &font = *context.Font;

    constexpr float titleScale = 1.8f;
    const float titleWidth = BitmapFont::MeasureTextWidth(title, titleScale);
    BitmapFont::DrawString(
        renderer,
        (static_cast<float>(Constants::WindowWidth) - titleWidth) * 0.5f,
        70.0f,
        title,
        kWhite,
        titleScale
    );

    ButtonStyle style{};
    style.TextScale = 0.85f;

    for (int index = 0; index < static_cast<int>(buttons.size()); ++index) {
        buttons[static_cast<std::size_t>(index)].Draw(renderer, font, style, index == selectedIndex);
    }

    BitmapFont::DrawString(renderer, 390.0f, 560.0f, "USE W S OR ARROWS AND ENTER", kMuted, 0.72f);
}

const PongGame::DifficultyTuning &PongGame::GetDifficultyTuning() const noexcept {
    static constexpr std::array difficultyTable
    {
        DifficultyTuning{
            "EASY",
            380.0f,
            160.0f,
            340.0f,
            1.30f,
            0.30f,
            0.75f,
            90.0f,
            0.48f
        },
        DifficultyTuning{
            "MEDIUM",
            500.0f,
            120.0f,
            500.0f,
            0.75f,
            0.20f,
            0.55f,
            48.0f,
            0.68f
        },
        DifficultyTuning{
            "HARD",
            650.0f,
            96.0f,
            760.0f,
            0.30f,
            0.12f,
            0.30f,
            14.0f,
            0.86f
        }
    };

    return difficultyTable[static_cast<std::size_t>(m_difficulty)];
}

float PongGame::GetPlayableTop() noexcept {
    return FieldMarginTop;
}

float PongGame::GetPlayableBottom() noexcept {
    return static_cast<float>(Constants::WindowHeight) - FieldMarginBottom;
}

float PongGame::GetBallSpeed() const noexcept {
    return m_ball.Movement.Velocity.Length();
}

std::string PongGame::BuildScoreText() const {
    std::ostringstream stream;
    if (m_leftScore < 10) {
        stream << '0';
    }
    stream << m_leftScore << " : ";
    if (m_rightScore < 10) {
        stream << '0';
    }
    stream << m_rightScore;
    return stream.str();
}

std::string PongGame::BuildModeText() const {
    return m_gameMode == GameMode::TwoPlayers ? "MODE PVP" : "MODE BOT";
}

std::string PongGame::BuildDifficultyText() const {
    return std::string{"LEVEL "} + GetDifficultyTuning().Label;
}

std::string PongGame::BuildFpsText() const {
    std::ostringstream stream;
    stream << "FPS " << m_displayFps;
    return stream.str();
}
