//
// Created by SyperOlao on 25.03.2026.
//

#ifndef PINGPONG_BOUNDINGSPHERE_H
#define PINGPONG_BOUNDINGSPHERE_H
#include <SimpleMath.h>

struct BoundingSphere final {
    DirectX::SimpleMath::Vector3 Center{0.0f, 0.0f, 0.0f};
    float Radius{0.0f};

    [[nodiscard]] bool Intersects(const BoundingSphere &other) const noexcept;
};
#endif //PINGPONG_BOUNDINGSPHERE_H
