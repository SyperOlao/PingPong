// Created by SyperOlao on 25.03.2026.
#ifndef PINGPONG_SCENERENDERER3D_H
#define PINGPONG_SCENERENDERER3D_H

#include <SimpleMath.h>

class ModelRenderer;
class PrimitiveRenderer3D;

#include "Core/Graphics/Rendering/Renderables/SceneRenderData3D.h"

class SceneRenderer3D final
{
public:
    void Initialize(PrimitiveRenderer3D &primitiveRenderer, ModelRenderer &modelRenderer) noexcept;

    void RenderScene(const SceneRenderData3D &sceneRenderData) const;

private:
    PrimitiveRenderer3D *m_primitiveRenderer{nullptr};
    ModelRenderer *m_modelRenderer{nullptr};
};

#endif