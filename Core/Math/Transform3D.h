//
// Created by SyperOlao on 19.03.2026.
//

#ifndef PINGPONG_TRANSFORM3D_H
#define PINGPONG_TRANSFORM3D_H
#include <SimpleMath.h>

struct Transform3D final {
    DirectX::SimpleMath::Vector3 Position{0.0f, 0.0f, 0.0f};
    DirectX::SimpleMath::Vector3 RotationEulerRad{0.0f, 0.0f, 0.0f};
    DirectX::SimpleMath::Vector3 Scale{1.0f, 1.0f, 1.0f};

    [[nodiscard]] DirectX::SimpleMath::Matrix GetWorldMatrix() const noexcept {
        using namespace DirectX::SimpleMath;

        return
            Matrix::CreateScale(Scale) *
            Matrix::CreateRotationX(RotationEulerRad.x) *
            Matrix::CreateRotationY(RotationEulerRad.y) *
            Matrix::CreateRotationZ(RotationEulerRad.z) *
            Matrix::CreateTranslation(Position);
    }
};

#endif //PINGPONG_TRANSFORM3D_H
