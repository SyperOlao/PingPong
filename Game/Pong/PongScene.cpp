//
// Created by SyperOlao on 17.03.2026.
//

#include "Game/Pong/PongScene.h"

#include "Core/App/AppContext.h"


void PongScene::Initialize() noexcept
{
    m_playField.Initialize();
}

void PongScene::RenderGameplay(
    const AppContext& context,
    const Paddle& leftPaddle,
    const Paddle& rightPaddle,
    const Ball& ball,
    const ScoreType leftScore,
    const ScoreType rightScore,
    const int fps,
    const GameMode gameMode,
    const Difficulty difficulty
) const
{
    auto& renderer = context.GetShapeRenderer2D();

    m_playField.Render(renderer, leftPaddle, rightPaddle, ball);
    ScoreBoard::Render(renderer, leftScore, rightScore, fps, gameMode, difficulty);
}