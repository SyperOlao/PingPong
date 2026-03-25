//
// Created by SyperOlao on 19.03.2026.
//
// Same TRS composition and axis convention as SpatialMath::ComposeWorldMatrixTrs.

#ifndef PINGPONG_TRANSFORM3D_H
#define PINGPONG_TRANSFORM3D_H

#include <SimpleMath.h>

struct Transform3D final {
    DirectX::SimpleMath::Vector3 Position{0.0f, 0.0f, 0.0f};
    DirectX::SimpleMath::Vector3 RotationEulerRad{0.0f, 0.0f, 0.0f};
    DirectX::SimpleMath::Vector3 Scale{1.0f, 1.0f, 1.0f};

    [[nodiscard]] DirectX::SimpleMath::Matrix GetWorldMatrix() const noexcept;

    [[nodiscard]] DirectX::SimpleMath::Matrix GetRotationMatrix() const noexcept;

    [[nodiscard]] DirectX::SimpleMath::Vector3 GetForward() const noexcept;

    [[nodiscard]] DirectX::SimpleMath::Vector3 GetRight() const noexcept;

    [[nodiscard]] DirectX::SimpleMath::Vector3 GetUp() const noexcept;
};

#endif //PINGPONG_TRANSFORM3D_H
