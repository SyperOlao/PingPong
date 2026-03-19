//
// Created by SyperOlao on 17.03.2026.
//

#ifndef PINGPONG_COLLISIONSYSTEM_H
#define PINGPONG_COLLISIONSYSTEM_H
#include <SimpleMath.h>

#include "AABB.h"
#include "../Common/Types.h"

struct CollisionManifold final
{
    bool HasCollision{false};
    DirectX::SimpleMath::Vector2 Normal{0.0f, 0.0f};
    float Penetration{0.0f};
};

class CollisionSystem final
{
public:
    [[nodiscard]] static bool CheckCollision(const AABB& a, const AABB& b) noexcept;
    [[nodiscard]] static CollisionManifold FindCollision(const AABB& a, const AABB& b) noexcept;


    static void ReflectAgainstNormal(
        DirectX::SimpleMath::Vector2& velocity,
        const DirectX::SimpleMath::Vector2& normal
    ) noexcept;

    /**
     * Строит новую скорость мяча после удара о ракетку.
     * bounceToRight = true для левой ракетки, false для правой.
     */
    [[nodiscard]] static DirectX::SimpleMath::Vector2 BuildPaddleBounceVelocity(
        const AABB& ball,
        const AABB& paddle,
        float speed,
        float maxBounceAngleRadians,
        bool bounceToRight
    ) noexcept;

    /**
     * Возвращает сторону, через которую мяч ВЫШЕЛ.
     * Это не "кто забил", а именно "куда улетел".
     */
    [[nodiscard]] static CourtSide CheckOutOfBounds(
        const AABB& ball,
        float leftBoundary,
        float rightBoundary
    ) noexcept;

};


#endif //PINGPONG_COLLISIONSYSTEM_H