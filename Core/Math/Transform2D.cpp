//
// Created by SyperOlao on 25.03.2026.
//

#include "Core/Math/Transform2D.h"

DirectX::SimpleMath::Matrix Transform2D::GetWorldMatrixXyPlane() const noexcept {
    using namespace DirectX::SimpleMath;

    return Matrix::CreateScale(Scale.x, Scale.y, 1.0f) * Matrix::CreateRotationZ(RotationRadians) *
           Matrix::CreateTranslation(Position.x, Position.y, 0.0f);
}
