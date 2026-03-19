//
// Created by SyperOlao on 18.03.2026.
//

#ifndef MYPROJECT_PONGCOLLISIONSYSTEM_H
#define MYPROJECT_PONGCOLLISIONSYSTEM_H

#include "Core/Common/Types.h"

class Ball;
class Paddle;

namespace PongCollisionSystem
{
    [[nodiscard]] bool HandleWallCollision(Ball& ball, float playableTop, float playableBottom) noexcept;
    [[nodiscard]] bool HandlePaddleCollision(Ball& ball, const Paddle& leftPaddle, const Paddle& rightPaddle);
    [[nodiscard]] CourtSide CheckScoring(const Ball& ball) noexcept;
}


#endif //MYPROJECT_PONGCOLLISIONSYSTEM_H