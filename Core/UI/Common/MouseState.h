//
// Created by SyperOlao on 25.03.2026.
//

#ifndef PINGPONG_MOUSESTATE_H
#define PINGPONG_MOUSESTATE_H

struct MouseState final
{
    float X{0.0f};
    float Y{0.0f};

    bool LeftDown{false};
    bool LeftPressed{false};
    bool LeftReleased{false};
};

#endif //PINGPONG_MOUSESTATE_H