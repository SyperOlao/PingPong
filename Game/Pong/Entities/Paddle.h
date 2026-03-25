//
// Created by SyperOlao on 18.03.2026.
//

#ifndef MYPROJECT_PADDLE_H
#define MYPROJECT_PADDLE_H
#include "Core/Graphics/Color.h"
#include "Core/Math/DXIncludes.h"
#include "Core/Physics/AABB.h"
#include "Game/Pong/Components/Collider2D.h"
#include "Game/Pong/Components/MovementComponent.h"
#include "Core/Math/Transform2D.h"

class Paddle {
public:
    Transform2D Transform{};
    MovementComponent Movement{};
    Collider2D Collider{};
    Color ColorTint{1.0f, 1.0f, 1.0f, 1.0f};

public:
    void SetDimensions(float width, float height) noexcept;

    [[nodiscard]] float Width() const noexcept;
    [[nodiscard]] float Height() const noexcept;
    [[nodiscard]] AABB GetAABB() const noexcept;
};




#endif //MYPROJECT_PADDLE_H
