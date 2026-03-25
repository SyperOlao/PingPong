//
// Created by SyperOlao on 17.03.2026.
//

#include "../Entities/ScoreBoard.h"
#include <string>

#include "Game/Pong/Common/Constants.h"
#include "Core/Graphics/Color.h"
#include "../../../Core/Graphics/Rendering/ShapeRenderer2D.h"
#include "Core/UI/BitmapFont.h"
#include "Game/Pong/Systems/PongRules.h"

namespace {
    constexpr Color kWhite{1.0f, 1.0f, 1.0f, 1.0f};
    constexpr Color kMuted{0.65f, 0.68f, 0.78f, 1.0f};
    constexpr Color kAccent{0.82f, 0.84f, 0.93f, 1.0f};
}

void ScoreBoard::Render(
    const ShapeRenderer2D &renderer,
    const ScoreType leftScore,
    const ScoreType rightScore,
    const int fps,
    const GameMode gameMode,
    const Difficulty difficulty
) {
    const std::string scoreText = PongRules::BuildScoreText(leftScore, rightScore);
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

    BitmapFont::DrawString(
        renderer,
        24.0f,
        24.0f,
        "FPS " + std::to_string(fps),
        kAccent,
        0.9f
    );

    BitmapFont::DrawString(
        renderer,
        24.0f,
        58.0f,
        PongRules::BuildModeText(gameMode),
        kMuted,
        0.75f
    );

    BitmapFont::DrawString(
        renderer,
        24.0f,
        86.0f,
        PongRules::BuildDifficultyText(difficulty),
        kMuted,
        0.75f
    );

    BitmapFont::DrawString(
        renderer,
        24.0f,
        114.0f,
        "ESC MENU  ENTER RESET",
        kMuted,
        0.68f
    );
}
