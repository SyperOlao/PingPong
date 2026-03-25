//
// Created by SyperOlao on 25.03.2026.
//

#ifndef PINGPONG_MODELRENDERER_H
#define PINGPONG_MODELRENDERER_H
#include <SimpleMath.h>

class GraphicsDevice;
class Camera;
class ModelAsset;

class ModelRenderer final
{
public:
    void Initialize(GraphicsDevice& graphics);
    void DrawModel(
        const ModelAsset& model,
        const DirectX::SimpleMath::Matrix& world,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& projection
    ) const;

private:
    GraphicsDevice* m_graphics{nullptr};
};
#endif //PINGPONG_MODELRENDERER_H