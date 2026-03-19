//
// Created by SyperOlao on 18.03.2026.
//
#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "PongCollisionSystem.h"

#include <algorithm>
#include <cmath>

#include "Core/Common/Constants.h"
#include "Core/Math/MathHelpers.h"
#include "Core/Physics/CollisionSystem.h"
#include "Game/Pong/Entities/Ball.h"
#include "Game/Pong/Entities/Paddle.h"

namespace {
    DirectX::SimpleMath::Vector2 BuildSharperPaddleBounceVelocity(
        const AABB &ballAABB,
        const AABB &paddleAABB,
        const float speed,
        const bool isLeftPaddle
    ) noexcept {
        const float paddleCenterY = (paddleAABB.Min.y + paddleAABB.Max.y) * 0.5f;
        const float ballCenterY = (ballAABB.Min.y + ballAABB.Max.y) * 0.5f;

        const float halfPaddleHeight = std::max((paddleAABB.Max.y - paddleAABB.Min.y) * 0.5f, 0.0001f);

        float offset = (ballCenterY - paddleCenterY) / halfPaddleHeight;
        offset = MathHelpers::Clamp(offset, -1.0f, 1.0f);

        constexpr float kVerticalInfluence = 1.35f;

        const float directionX = isLeftPaddle ? 1.0f : -1.0f;

        DirectX::SimpleMath::Vector2 direction{
            directionX,
            offset * kVerticalInfluence
        };

        direction = MathHelpers::SafeNormalize(
            direction,
            DirectX::SimpleMath::Vector2{directionX, 0.0f}
        );

        return direction * speed;
    }
}

namespace PongCollisionSystem {
    bool PongCollisionSystem::HandleWallCollision(
        Ball &ball,
        const float playableTop,
        const float playableBottom
    ) noexcept {
        const AABB ballAABB = ball.GetAABB();

        if (ballAABB.Min.y <= playableTop) {
            ball.Transform.Position.y = playableTop;
            ball.Movement.Velocity.y = std::abs(ball.Movement.Velocity.y);
            return true;
        }

        if (ballAABB.Max.y >= playableBottom) {
            ball.Transform.Position.y = playableBottom - ball.Size();
            ball.Movement.Velocity.y = -std::abs(ball.Movement.Velocity.y);
            return true;
        }

        return false;
    }

    bool PongCollisionSystem::HandlePaddleCollision(
        Ball &ball,
        const Paddle &leftPaddle,
        const Paddle &rightPaddle
    ) {
        const AABB ballAABB = ball.GetAABB();
        const AABB leftPaddleAABB = leftPaddle.GetAABB();
        const AABB rightPaddleAABB = rightPaddle.GetAABB();

        if (CollisionSystem::CheckCollision(ballAABB, leftPaddleAABB) && ball.Movement.Velocity.x < 0.0f) {
            ball.Transform.Position.x = leftPaddleAABB.Max.x + 0.5f;

            const float nextSpeed = std::min(
                ball.Movement.Velocity.Length() * Constants::BallSpeedMultiplierOnPaddleHit,
                Constants::BallMaxSpeed
            );

            ball.Movement.Velocity = BuildSharperPaddleBounceVelocity(
                ball.GetAABB(),
                leftPaddleAABB,
                nextSpeed,
                true
            );

            return true;
        }

        if (CollisionSystem::CheckCollision(ballAABB, rightPaddleAABB) && ball.Movement.Velocity.x > 0.0f) {
            ball.Transform.Position.x = rightPaddleAABB.Min.x - ball.Size() - 0.5f;

            const float nextSpeed = std::min(
                ball.Movement.Velocity.Length() * Constants::BallSpeedMultiplierOnPaddleHit,
                Constants::BallMaxSpeed
            );

            ball.Movement.Velocity = BuildSharperPaddleBounceVelocity(
                ball.GetAABB(),
                rightPaddleAABB,
                nextSpeed,
                false
            );

            return true;
        }

        return false;
    }

    CourtSide CheckScoring(const Ball &ball) noexcept {
        return CollisionSystem::CheckOutOfBounds(
            ball.GetAABB(),
            0.0f,
            static_cast<float>(Constants::WindowWidth)
        );
    }
}
