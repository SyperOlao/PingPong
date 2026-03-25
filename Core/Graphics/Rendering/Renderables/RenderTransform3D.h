#ifndef PINGPONG_RENDERTRANSFORM3D_H
#define PINGPONG_RENDERTRANSFORM3D_H

#include <SimpleMath.h>

#include "Core/Math/Transform3D.h"

struct RenderTransform3D final
{
    Transform3D Transform{};

    [[nodiscard]] DirectX::SimpleMath::Matrix GetWorldMatrix() const noexcept
    {
        return Transform.GetWorldMatrix();
    }
};

#endif
