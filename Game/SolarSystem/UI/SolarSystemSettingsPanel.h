//
// Created by SyperOlao on 25.03.2026.
//

#ifndef PINGPONG_SOLARSYSTEMSETTINGSPANEL_H
#define PINGPONG_SOLARSYSTEMSETTINGSPANEL_H

#include "Core/UI/Common/MouseState.h"
#include "Core/UI/Common/RectF.h"
#include "Core/UI/Common/UITheme.h"
#include "Core/UI/Widgets/CollapsibleSection.h"
#include "Core/UI/Widgets/SliderWidget.h"
#include "Game/SolarSystem/Systems/SolarSystemTuning.h"

class ShapeRenderer2D;

class SolarSystemSettingsPanel final
{
public:
    void Initialize();
    void SetBounds(const RectF& bounds) noexcept;

    void Update(const MouseState& mouse);
    void Render(ShapeRenderer2D& shapeRenderer) const;

    [[nodiscard]] SolarSystemTuning GetTuning() const noexcept;

    [[nodiscard]] bool IsOpen() const noexcept;
    void Toggle() noexcept;

private:
    void UpdateLayout() noexcept;

private:
    bool m_isOpen{true};
    RectF m_bounds{20.0f, 150.0f, 300.0f, 320.0f};

    UITheme m_theme{};

    CollapsibleSection m_dynamicsSection;
    CollapsibleSection m_orbitSection;

    SliderWidget m_planetRotationSlider;
    SliderWidget m_moonRotationSlider;
    SliderWidget m_planetOrbitSlider;
    SliderWidget m_moonOrbitSlider;
    SliderWidget m_eccentricitySlider;
};

#endif //PINGPONG_SOLARSYSTEMSETTINGSPANEL_H