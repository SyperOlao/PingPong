#ifndef PINGPONG_MODELRENDERER_H
#define PINGPONG_MODELRENDERER_H

#include <SimpleMath.h>
#include <directxtk/CommonStates.h>
#include <memory>

#include "Core/Graphics/Rendering/Renderables/RenderMaterialParameters.h"

class GraphicsDevice;
class ModelAsset;

class ModelRenderer final
{
public:
    void Initialize(GraphicsDevice &graphics);

    void DrawModel(
        const ModelAsset &model,
        const DirectX::SimpleMath::Matrix &world,
        const DirectX::SimpleMath::Matrix &view,
        const DirectX::SimpleMath::Matrix &projection,
        const RenderMaterialParameters &material
    ) const;

private:
    GraphicsDevice *m_graphics{nullptr};
    std::unique_ptr<DirectX::CommonStates> m_commonStates{};
};

#endif
