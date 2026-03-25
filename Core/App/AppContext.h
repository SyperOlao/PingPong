//
// Created by SyperOlao on 18.03.2026.
//

#ifndef PINGPONG_APPCONTEXT_H
#define PINGPONG_APPCONTEXT_H

#include "EngineServices.h"

class ShapeRenderer2D;
class PrimitiveRenderer3D;
class ModelRenderer;
class DebugDrawQueue;
class FrameRenderer;
class DeferredFrameResources;

struct AppContext final {
    PlatformServices Platform{};
    InputServices Input{};
    GraphicsServices Graphics{};
    UiServices Ui{};
    AudioServices Audio{};
    AssetServices Assets{};

    [[nodiscard]] bool IsValid() const noexcept;

    [[nodiscard]] ShapeRenderer2D &GetShapeRenderer2D() const;

    [[nodiscard]] PrimitiveRenderer3D &GetPrimitiveRenderer3D() const;

    [[nodiscard]] ModelRenderer &GetModelRenderer() const;

    [[nodiscard]] DebugDrawQueue &GetDebugDraw() const;

    [[nodiscard]] FrameRenderer &GetFrameRenderer() const;

    [[nodiscard]] DeferredFrameResources &GetDeferredFrameResources() const;

    void ResizeDeferredResources() const;
};

#endif


