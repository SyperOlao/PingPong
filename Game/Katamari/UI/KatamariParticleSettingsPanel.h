#ifndef PINGPONG_KATAMARIPARTICLESETTINGSPANEL_H
#define PINGPONG_KATAMARIPARTICLESETTINGSPANEL_H

#include "Core/Graphics/Particles/GpuParticleTypes.h"
#include "Core/UI/Common/MouseState.h"
#include "Core/UI/Common/RectF.h"
#include "Core/UI/Common/UITheme.h"
#include "Core/UI/Widgets/CollapsibleSection.h"
#include "Core/UI/Widgets/SliderWidget.h"

#include <cstdint>

class ShapeRenderer2D;

enum class KatamariParticleUiAction : std::uint8_t
{
    None = 0,
    SliderMoved
};

class KatamariParticleSettingsPanel final
{
public:
    void Initialize(const GpuParticleEmitterDesc &emitterDesc, float playfieldHalfExtent);

    void SetBounds(const RectF &bounds) noexcept;

    [[nodiscard]] KatamariParticleUiAction Update(const MouseState &mouse);

    void Render(ShapeRenderer2D &shapeRenderer) const;

    void ApplyToEmitterDesc(GpuParticleEmitterDesc &emitterDesc, float playfieldHalfExtent) const noexcept;

    void SetValuesFromEmitterDesc(const GpuParticleEmitterDesc &emitterDesc, float playfieldHalfExtent) noexcept;

    [[nodiscard]] bool IsOpen() const noexcept;

    [[nodiscard]] bool IsInteracting() const noexcept;

    void Toggle() noexcept;

private:
    void UpdateLayout() noexcept;

    [[nodiscard]] float CalculateContentHeight() const noexcept;

private:
    bool m_isOpen{false};
    RectF m_bounds{290.0f, 52.0f, 390.0f, 580.0f};

    UITheme m_theme{};

    CollapsibleSection m_spawnSection{};
    CollapsibleSection m_drawSection{};

    SliderWidget m_spawnRateSlider{};
    SliderWidget m_spawnVolumeScaleSlider{};
    SliderWidget m_spawnCenterHeightSlider{};
    SliderWidget m_spawnHalfHeightSlider{};
    SliderWidget m_lifetimeSlider{};
    SliderWidget m_startSizeSlider{};
    SliderWidget m_drawAlphaScaleSlider{};
    SliderWidget m_drawSizeScaleSlider{};
};

#endif
