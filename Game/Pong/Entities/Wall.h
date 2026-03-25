//
// Created by SyperOlao on 18.03.2026.
//

#ifndef MYPROJECT_WALL_H
#define MYPROJECT_WALL_H

#include "Core/Graphics/Color.h"
#include "../../../Core/Graphics/Rendering/ShapeRenderer2D.h"
class ShapeRenderer2D;


class Wall final
{
public:
    void SetRect(const RectF& rect) noexcept;
    void SetColor(const Color& color) noexcept;

    [[nodiscard]] const RectF& GetRect() const noexcept;

    void Render(const ShapeRenderer2D& renderer) const;

private:
    RectF m_rect{};
    Color m_color{1.0f, 1.0f, 1.0f, 1.0f};
};

#endif //MYPROJECT_WALL_H