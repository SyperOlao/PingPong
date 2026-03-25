#include "Core/Graphics/ModelRenderer.h"

#include "Core/Graphics/GraphicsDevice.h"
#include "Core/Graphics/ModelAsset.h"

#include <directxtk/Effects.h>
#include <directxtk/Model.h>

#include <stdexcept>

static void ApplyTintToEffect(
    DirectX::IEffect *EffectTarget,
    const DirectX::XMVECTOR &ColorAndAlpha
)
{
    if (DirectX::BasicEffect *const EffectBasic = dynamic_cast<DirectX::BasicEffect *>(EffectTarget))
    {
        EffectBasic->SetColorAndAlpha(ColorAndAlpha);
        return;
    }

    if (DirectX::AlphaTestEffect *const EffectAlphaTest = dynamic_cast<DirectX::AlphaTestEffect *>(EffectTarget))
    {
        EffectAlphaTest->SetColorAndAlpha(ColorAndAlpha);
        return;
    }

    if (DirectX::DualTextureEffect *const EffectDualTexture = dynamic_cast<DirectX::DualTextureEffect *>(EffectTarget))
    {
        EffectDualTexture->SetDiffuseColor(ColorAndAlpha);
        EffectDualTexture->SetAlpha(DirectX::XMVectorGetW(ColorAndAlpha));
        return;
    }

    if (DirectX::EnvironmentMapEffect *const EffectEnvironmentMap = dynamic_cast<DirectX::EnvironmentMapEffect *>(EffectTarget))
    {
        EffectEnvironmentMap->SetColorAndAlpha(ColorAndAlpha);
        return;
    }

    if (DirectX::SkinnedEffect *const EffectSkinned = dynamic_cast<DirectX::SkinnedEffect *>(EffectTarget))
    {
        EffectSkinned->SetColorAndAlpha(ColorAndAlpha);
        return;
    }

    if (DirectX::NormalMapEffect *const EffectNormalMap = dynamic_cast<DirectX::NormalMapEffect *>(EffectTarget))
    {
        EffectNormalMap->SetColorAndAlpha(ColorAndAlpha);
        return;
    }
}

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
    if (m_graphics == nullptr || m_commonStates == nullptr)
    {
        throw std::logic_error("ModelRenderer::DrawModel called before Initialize.");
    }

    DirectX::Model *const DrawableModel = model.Get();
    if (DrawableModel == nullptr)
    {
        return;
    }

    ID3D11DeviceContext *const DeviceContext = m_graphics->GetImmediateContext();
    if (DeviceContext == nullptr)
    {
        throw std::logic_error("ModelRenderer::DrawModel failed: null device context.");
    }

    m_graphics->BindMainRenderTargets();

    const DirectX::XMFLOAT4 &BaseColorFloat4 = static_cast<const DirectX::XMFLOAT4 &>(material.BaseColor);
    const DirectX::XMVECTOR ColorAndAlpha = DirectX::XMLoadFloat4(&BaseColorFloat4);

    DrawableModel->UpdateEffects(
        [&](DirectX::IEffect *EffectTarget)
        {
            ApplyTintToEffect(EffectTarget, ColorAndAlpha);
        }
    );

    DrawableModel->Draw(
        DeviceContext,
        *m_commonStates,
        world,
        view,
        projection,
        material.Wireframe
    );
}
