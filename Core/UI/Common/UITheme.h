//
// Created by SyperOlao on 25.03.2026.
//

#ifndef PINGPONG_UITHEME_H
#define PINGPONG_UITHEME_H
#include "Core/Graphics/Color.h"
#include "Game/SolarSystem/SolarSystemGame.h"

struct UITheme final {
    Color PanelBackground{0.05f, 0.07f, 0.11f, 0.92f};
    Color PanelBorder{0.20f, 0.28f, 0.38f, 1.0f};

    Color HeaderBackground{0.10f, 0.14f, 0.20f, 1.0f};
    Color HeaderText{0.88f, 0.92f, 0.98f, 1.0f};

    Color SliderTrack{0.18f, 0.24f, 0.34f, 1.0f};
    Color SliderFill{0.22f, 0.60f, 0.95f, 1.0f};
    Color SliderHandle{0.90f, 0.95f, 1.0f, 1.0f};
    Color SliderLabel{0.82f, 0.88f, 0.96f, 1.0f};
    Color SliderValue{0.70f, 0.82f, 0.98f, 1.0f};

    float PanelPadding{14.0f};
    float SectionSpacing{10.0f};
    float WidgetSpacing{8.0f};

    float HeaderHeight{24.0f};
    float SliderHeight{34.0f};
    float SliderTrackHeight{6.0f};
    float SliderHandleWidth{12.0f};
};

#endif //PINGPONG_UITHEME_H
