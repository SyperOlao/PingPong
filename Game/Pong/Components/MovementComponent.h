//
// Created by SyperOlao on 17.03.2026.
//

#ifndef PINGPONG_MOVEMENTCOMPONENT_H
#define PINGPONG_MOVEMENTCOMPONENT_H

#include <SimpleMath.h>

#include "Core/Math/Transform2D.h"

struct MovementComponent final {
    DirectX::SimpleMath::Vector2 Velocity{0.0f, 0.0f};
    DirectX::SimpleMath::Vector2 Acceleration{0.0f, 0.0f};
    bool Enabled{true};

    void Update(Transform2D &transform, const float deltaTime) noexcept {
        if (!Enabled) {
            return;
        }

        Velocity += Acceleration * deltaTime;
        transform.Position += Velocity * deltaTime;
    }

    void Stop() noexcept {
        Velocity = DirectX::SimpleMath::Vector2{0.0f, 0.0f};
        Acceleration = DirectX::SimpleMath::Vector2{0.0f, 0.0f};
    }
};

#endif //PINGPONG_MOVEMENTCOMPONENT_H
