//
// Created by SyperOlao on 18.03.2026.
//

#ifndef MYPROJECT_BALL_H
#define MYPROJECT_BALL_H
#include "Core/Graphics/Color.h"
#include "Core/Math/DXIncludes.h"
#include "Core/Physics/AABB.h"
#include "Game/Pong/Components/Collider2D.h"
#include "Game/Pong/Components/MovementComponent.h"
#include "Core/Math/Transform2D.h"

class Ball final{
public:
    Transform2D Transform{};
    MovementComponent Movement{};
    Collider2D Collider{};
    Color ColorTint{1.0f, 1.0f, 1.0f, 1.0f};

public:
    void SetSize(float size) noexcept;

    [[nodiscard]] float Size() const noexcept;
    [[nodiscard]] AABB GetAABB() const noexcept;
};


#endif //MYPROJECT_BALL_H