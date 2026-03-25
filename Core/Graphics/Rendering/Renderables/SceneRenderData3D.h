// Created by SyperOlao on 25.03.2026.
#ifndef PINGPONG_SCENERENDERDATA3D_H
#define PINGPONG_SCENERENDERDATA3D_H

#include <vector>

#include <SimpleMath.h>

#include "Core/Graphics/Rendering/Renderables/Renderable3D.h"

struct SceneRenderData3D final
{
    DirectX::SimpleMath::Matrix View{DirectX::SimpleMath::Matrix::Identity};
    DirectX::SimpleMath::Matrix Projection{DirectX::SimpleMath::Matrix::Identity};
    std::vector<Renderable3D> Renderables{};
};

#endif