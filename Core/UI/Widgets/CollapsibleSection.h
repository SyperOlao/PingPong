//
// Created by SyperOlao on 25.03.2026.
//

#ifndef PINGPONG_COLLAPSIBLESECTION_H
#define PINGPONG_COLLAPSIBLESECTION_H

#include <string>

#include "Core/UI/Common/RectF.h"
#include "Core/UI/Common/MouseState.h"
#include "Core/UI/Common/UITheme.h"

class ShapeRenderer2D;

class CollapsibleSection final
{
public:
    void SetBounds(const RectF& bounds) noexcept;
    void SetTitle(std::string title);
    void SetExpanded(bool isExpanded) noexcept;
    void SetTheme(const UITheme* theme) noexcept;

    void Update(const MouseState& mouse);
    void Render(ShapeRenderer2D& shapeRenderer) const;

    [[nodiscard]] bool IsExpanded() const noexcept;
    [[nodiscard]] const RectF& GetBounds() const noexcept;
    [[nodiscard]] const std::string& GetTitle() const noexcept;

private:
    RectF m_bounds{};
    std::string m_title{"Section"};
    bool m_isExpanded{true};
    const UITheme* m_theme{nullptr};
};

#endif //PINGPONG_COLLAPSIBLESECTION_H