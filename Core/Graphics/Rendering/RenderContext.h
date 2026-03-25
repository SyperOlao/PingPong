//
// Created by SyperOlao on 25.03.2026.
//

#ifndef PINGPONG_RENDERCONTEXT_H
#define PINGPONG_RENDERCONTEXT_H
#include "Core/Graphics/GraphicsDevice.h"
#include "PrimitiveRenderer3D.h"
#include "ShapeRenderer2D.h"


class RenderContext final {
public:
    RenderContext() = default;

    ~RenderContext() = default;

    RenderContext(const RenderContext &) = delete;

    RenderContext &operator=(const RenderContext &) = delete;

    RenderContext(RenderContext &&) = delete;

    RenderContext &operator=(RenderContext &&) = delete;

    void Initialize(GraphicsDevice &graphics);

    void BindDefaultTargets() const;

    [[nodiscard]] ShapeRenderer2D &GetShapeRenderer2D() noexcept;

    [[nodiscard]] const ShapeRenderer2D &GetShapeRenderer2D() const noexcept;

    [[nodiscard]] PrimitiveRenderer3D &GetPrimitiveRenderer3D() noexcept;

    [[nodiscard]] const PrimitiveRenderer3D &GetPrimitiveRenderer3D() const noexcept;

private:
    GraphicsDevice *m_graphics{nullptr};
    ShapeRenderer2D m_shapeRenderer2D;
    PrimitiveRenderer3D m_primitiveRenderer3D;
};

#endif //PINGPONG_RENDERCONTEXT_H
