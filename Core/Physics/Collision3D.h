//
// Created by SyperOlao on 25.03.2026.
//

#ifndef PINGPONG_COLLISION3D_H
#define PINGPONG_COLLISION3D_H
#include "BoundingSphere.h"

struct SphereContact final {
    bool HasCollision{false};
    DirectX::SimpleMath::Vector3 Normal{0.0f, 1.0f, 0.0f};
    float Penetration{0.0f};
};

class Collision3D final {
public:
    [[nodiscard]] static bool CheckSphereSphere(
        const BoundingSphere &a,
        const BoundingSphere &b
    ) noexcept;

    [[nodiscard]] static SphereContact FindSphereSphereContact(
        const BoundingSphere &a,
        const BoundingSphere &b
    ) noexcept;
};


#endif //PINGPONG_COLLISION3D_H
