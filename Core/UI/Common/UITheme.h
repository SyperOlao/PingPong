//
// Created by SyperOlao on 25.03.2026.
//

#ifndef PINGPONG_UITHEME_H
#define PINGPONG_UITHEME_H
#include "Core/Graphics/Color.h"

struct UITheme final {
    Color PanelBackground{0.02f, 0.04f, 0.08f, 0.96f};
    Color PanelBorder{0.15f, 0.22f, 0.34f, 1.0f};

    Color HeaderBackground{0.08f, 0.12f, 0.19f, 1.0f};
    Color HeaderText{0.88f, 0.92f, 0.98f, 1.0f};

    Color SliderTrack{0.16f, 0.21f, 0.32f, 1.0f};
    Color SliderFill{0.23f, 0.63f, 1.0f, 1.0f};
    Color SliderHandle{0.93f, 0.97f, 1.0f, 1.0f};
    Color SliderHandleAccent{0.64f, 0.82f, 1.0f, 1.0f};
    Color SliderLabel{0.82f, 0.88f, 0.96f, 1.0f};
    Color SliderValue{0.60f, 0.80f, 1.0f, 1.0f};
    Color SliderCard{0.04f, 0.07f, 0.12f, 1.0f};

    float PanelPadding{14.0f};
    float SectionSpacing{12.0f};
    float WidgetSpacing{8.0f};

    float HeaderHeight{24.0f};
    float SliderHeight{48.0f};
    float SliderTrackHeight{8.0f};
    float SliderHandleWidth{14.0f};
};

#endif //PINGPONG_UITHEME_H
