//
// Created by SyperOlao on 17.03.2026.
//
#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "CollisionSystem.h"
#include <cmath>

#include "../Math/MathHelpers.h"

bool CollisionSystem::CheckCollision(const AABB &a, const AABB &b) noexcept {
    return a.Intersects(b);
}

CollisionManifold CollisionSystem::FindCollision(const AABB &a, const AABB &b) noexcept {
    CollisionManifold manifold{};
    if (!CheckCollision(a, b)) {
        return manifold;
    }

    const auto aCenter = a.GetCenter();
    const auto bCenter = b.GetCenter();
    const auto aExtents = a.GetExtents();
    const auto bExtents = b.GetExtents();

    const float deltaX = bCenter.x - aCenter.x;
    const float deltaY = bCenter.y - aCenter.y;

    const float overlapX = (aExtents.x + bExtents.x) - std::abs(deltaX);
    const float overlapY = (aExtents.y + bExtents.y) - std::abs(deltaY);

    manifold.HasCollision = true;

    if (overlapX < overlapY) {
        manifold.Penetration = overlapX;
        manifold.Normal = DirectX::SimpleMath::Vector2(
            deltaX < 0.0f ? -1.0f : 1.0f,
            0.0f
        );
    } else {
        manifold.Penetration = overlapY;
        manifold.Normal = DirectX::SimpleMath::Vector2(
            0.0f,
            deltaY < 0.0f ? -1.0f : 1.0f
        );
    }

    return manifold;
}

void CollisionSystem::ReflectAgainstNormal(
    DirectX::SimpleMath::Vector2 &velocity,
    const DirectX::SimpleMath::Vector2 &normal
) noexcept {
    velocity = MathHelpers::Reflect(velocity, normal);
}

DirectX::SimpleMath::Vector2 CollisionSystem::BuildPaddleBounceVelocity(
    const AABB &ball,
    const AABB &paddle,
    const float speed,
    const float maxBounceAngleRadians,
    const bool bounceToRight
) noexcept {
    const float paddleCenterY = paddle.GetCenter().y;
    const float ballCenterY = ball.GetCenter().y;
    const float halfPaddleHeight = std::max(paddle.GetExtents().y, 0.0001f);

    const float normalizedOffset = MathHelpers::Clamp(
        (ballCenterY - paddleCenterY) / halfPaddleHeight,
        -1.0f,
        1.0f
    );

    const float bounceAngle = normalizedOffset * maxBounceAngleRadians;
    const float horizontalDirection = bounceToRight ? 1.0f : -1.0f;

    DirectX::SimpleMath::Vector2 direction
    {
        std::cos(bounceAngle) * horizontalDirection,
        std::sin(bounceAngle)
    };

    direction = MathHelpers::SafeNormalize(direction, {horizontalDirection, 0.0f});
    return direction * speed;
}

CourtSide CollisionSystem::CheckOutOfBounds(
    const AABB &ball,
    const float leftBoundary,
    const float rightBoundary
) noexcept {
    if (ball.Max.x < leftBoundary) {
        return CourtSide::Left;
    }

    if (ball.Min.x > rightBoundary) {
        return CourtSide::Right;
    }

    return CourtSide::None;
}
DirectX::SimpleMath::Vector2 BuildSharperPaddleBounceVelocity(
       const AABB& ballAABB,
       const AABB& paddleAABB,
       const float speed,
       const bool isLeftPaddle
   )
{
    const float paddleCenterY = (paddleAABB.Min.y + paddleAABB.Max.y) * 0.5f;
    const float ballCenterY = (ballAABB.Min.y + ballAABB.Max.y) * 0.5f;

    float offset = (ballCenterY - paddleCenterY) / ((paddleAABB.Max.y - paddleAABB.Min.y) * 0.5f);
    offset = MathHelpers::Clamp(offset, -1.0f, 1.0f);

    constexpr float kVerticalInfluence = 1.25f;

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