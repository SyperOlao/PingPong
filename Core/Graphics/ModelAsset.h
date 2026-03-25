//
// Created by SyperOlao on 25.03.2026.
//

#ifndef PINGPONG_MODELASSET_H
#define PINGPONG_MODELASSET_H

#include <memory>
#include <string>
#include <directxtk/Model.h>

namespace DirectX
{
    inline namespace DX11
    {
        class IEffectFactory;
    }
}

class GraphicsDevice;

class ModelAsset final {
public:
    bool LoadFromCmo(GraphicsDevice &graphics, DirectX::IEffectFactory &effectFactory, const std::wstring &path);

    [[nodiscard]] DirectX::Model *Get() const noexcept;

private:
    std::unique_ptr<DirectX::Model> m_model;
};

#endif //PINGPONG_MODELASSET_H
