//
// Created by SyperOlao on 17.03.2026.
//

#ifndef PINGPONG_TRANSFORM2D_H
#define PINGPONG_TRANSFORM2D_H
#include "Core/Math/DXIncludes.h"

struct Transform2D final
{
    DirectX::SimpleMath::Vector2 Position{0.0f, 0.0f};
    DirectX::SimpleMath::Vector2 Scale{1.0f, 1.0f};
    float RotationRadians{0.0f};

    void Translate(const DirectX::SimpleMath::Vector2& delta) noexcept
    {
        Position += delta;
    }

    void SetPosition(const float x, const float y) noexcept
    {
        Position = DirectX::SimpleMath::Vector2{ x, y };
    }
};

#endif //PINGPONG_TRANSFORM2D_H