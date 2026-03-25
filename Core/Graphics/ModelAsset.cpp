//
// Created by SyperOlao on 25.03.2026.
//

#include "Core/Graphics/ModelAsset.h"

#include "Core/Graphics/GraphicsDevice.h"

#include <directxtk/Effects.h>
#include <directxtk/Model.h>

bool ModelAsset::LoadFromCmo(
    GraphicsDevice &graphics,
    DirectX::IEffectFactory &effectFactory,
    const std::wstring &path
) {
    m_model = DirectX::Model::CreateFromCMO(graphics.GetDevice(), path.c_str(), effectFactory);
    return m_model != nullptr;
}

DirectX::Model *ModelAsset::Get() const noexcept {
    return m_model.get();
}
