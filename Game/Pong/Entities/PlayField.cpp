//
// Created by SyperOlao on 17.03.2026.
//

#include "../Entities/PlayField.h"
#include "Game/Pong/Common/Constants.h"
#include "Core/Graphics/Color.h"
#include "../../../Core/Graphics/Rendering/ShapeRenderer2D.h"
#include "Game/Pong/Entities/Ball.h"
#include "Game/Pong/Entities/Paddle.h"
#include "Game/Pong/Systems/PongRules.h"

namespace {
    constexpr Color kMuted{0.65f, 0.68f, 0.78f, 1.0f};
}

void PlayField::Initialize() noexcept {
    UpdateLayout();
}

void PlayField::UpdateLayout() noexcept {
    const float playableTop = PongRules::GetPlayableTop();
    const float playableBottom = PongRules::GetPlayableBottom();
    const float playableHeight = playableBottom - playableTop;

    m_topWall.SetRect(
        RectF{
            0.0f,
            playableTop - 4.0f,
            static_cast<float>(Constants::WindowWidth),
            4.0f
        }
    );
    m_topWall.SetColor(kMuted);

    m_bottomWall.SetRect(
        RectF{
            0.0f,
            playableBottom,
            static_cast<float>(Constants::WindowWidth),
            4.0f
        }
    );
    m_bottomWall.SetColor(kMuted);

    const float centerLineX =
            static_cast<float>(Constants::WindowWidth) * 0.5f - Constants::CenterLineWidth * 0.5f;

    m_centerLine.Configure(centerLineX, playableTop, playableHeight);
}

void PlayField::Render(
    ShapeRenderer2D &renderer,
    const Paddle &leftPaddle,
    const Paddle &rightPaddle,
    const Ball &ball
) const {
    m_topWall.Render(renderer);
    m_bottomWall.Render(renderer);
    m_centerLine.Render(renderer);

    renderer.DrawFilledRect(
        leftPaddle.Transform.Position.x,
        leftPaddle.Transform.Position.y,
        leftPaddle.Width(),
        leftPaddle.Height(),
        leftPaddle.ColorTint
    );

    renderer.DrawFilledRect(
        rightPaddle.Transform.Position.x,
        rightPaddle.Transform.Position.y,
        rightPaddle.Width(),
        rightPaddle.Height(),
        rightPaddle.ColorTint
    );

    renderer.DrawFilledRect(
        ball.Transform.Position.x,
        ball.Transform.Position.y,
        ball.Size(),
        ball.Size(),
        ball.ColorTint
    );
}
