//
// Created by SyperOlao on 25.03.2026.
//

#include "Core/Graphics/ModelRenderer.h"

#include "Core/Graphics/GraphicsDevice.h"
#include "Core/Graphics/ModelAsset.h"

#include <directxtk/Model.h>

void ModelRenderer::Initialize(GraphicsDevice &graphics)
{
    m_graphics = &graphics;
    m_commonStates = std::make_unique<DirectX::CommonStates>(graphics.GetDevice());
}

void ModelRenderer::DrawModel(
    const ModelAsset &model,
    const DirectX::SimpleMath::Matrix &world,
    const DirectX::SimpleMath::Matrix &view,
    const DirectX::SimpleMath::Matrix &projection,
    const RenderMaterialParameters &material
) const
{
    (void)material;
    if (m_graphics == nullptr || m_commonStates == nullptr)
    {
        return;
    }

    const DirectX::Model *const drawable = model.Get();
    if (drawable == nullptr)
    {
        return;
    }

    ID3D11DeviceContext *const context = m_graphics->GetImmediateContext();
    if (context == nullptr)
    {
        return;
    }

    drawable->Draw(context, *m_commonStates, world, view, projection);
}
