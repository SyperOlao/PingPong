//
// Created by SyperOlao on 25.03.2026.
//

#ifndef PINGPONG_RECTF_H
#define PINGPONG_RECTF_H

struct RectF final
{
    float X{0.0f};
    float Y{0.0f};
    float Width{0.0f};
    float Height{0.0f};

    [[nodiscard]] float Left() const noexcept { return X; }
    [[nodiscard]] float Right() const noexcept { return X + Width; }
    [[nodiscard]] float Top() const noexcept { return Y; }
    [[nodiscard]] float Bottom() const noexcept { return Y + Height; }

    [[nodiscard]] bool Contains(const float px, const float py) const noexcept
    {
        return px >= Left() && px <= Right() &&
               py >= Top() && py <= Bottom();
    }
};



#endif //PINGPONG_RECTF_H