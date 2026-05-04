#include "Game/Katamari/UI/KatamariParticleSettingsPanel.h"

#include "Core/Graphics/Rendering/ShapeRenderer2D.h"
#include "Core/UI/BitmapFont.h"

#include <cmath>

namespace
{
constexpr float kSliderChangeEpsilon = 0.0001f;

[[nodiscard]] bool DidSliderValueChange(const float before, const float after) noexcept
{
    return std::fabs(before - after) > kSliderChangeEpsilon;
}

[[nodiscard]] float ResolveVolumeScale(
    const GpuParticleEmitterDesc &emitterDesc,
    const float playfieldHalfExtent
) noexcept
{
    if (playfieldHalfExtent <= 1.0e-5f)
    {
        return 0.05f;
    }

    return emitterDesc.SpawnVolumeHalfExtents.x / playfieldHalfExtent;
}
}

void KatamariParticleSettingsPanel::Initialize(
    const GpuParticleEmitterDesc &emitterDesc,
    const float playfieldHalfExtent
)
{
    m_spawnSection.SetTheme(&m_theme);
    m_spawnSection.SetTitle("Spawn");
    m_spawnSection.SetExpanded(true);

    m_drawSection.SetTheme(&m_theme);
    m_drawSection.SetTitle("Draw");
    m_drawSection.SetExpanded(true);

    m_spawnRateSlider.SetTheme(&m_theme);
    m_spawnRateSlider.SetLabel("Spawn rate");
    m_spawnRateSlider.SetRange(0.0f, 1200.0f);

    m_spawnVolumeScaleSlider.SetTheme(&m_theme);
    m_spawnVolumeScaleSlider.SetLabel("Volume XZ scale");
    m_spawnVolumeScaleSlider.SetRange(0.05f, 1.5f);

    m_spawnCenterHeightSlider.SetTheme(&m_theme);
    m_spawnCenterHeightSlider.SetLabel("Center height");
    m_spawnCenterHeightSlider.SetRange(0.0f, 80.0f);

    m_spawnHalfHeightSlider.SetTheme(&m_theme);
    m_spawnHalfHeightSlider.SetLabel("Half height");
    m_spawnHalfHeightSlider.SetRange(0.1f, 30.0f);

    m_lifetimeSlider.SetTheme(&m_theme);
    m_lifetimeSlider.SetLabel("Lifetime");
    m_lifetimeSlider.SetRange(0.2f, 12.0f);

    m_startSizeSlider.SetTheme(&m_theme);
    m_startSizeSlider.SetLabel("Start size");
    m_startSizeSlider.SetRange(0.02f, 2.0f);

    m_drawAlphaScaleSlider.SetTheme(&m_theme);
    m_drawAlphaScaleSlider.SetLabel("Alpha scale");
    m_drawAlphaScaleSlider.SetRange(0.0f, 2.0f);

    m_drawSizeScaleSlider.SetTheme(&m_theme);
    m_drawSizeScaleSlider.SetLabel("Size scale");
    m_drawSizeScaleSlider.SetRange(0.1f, 4.0f);

    SetValuesFromEmitterDesc(emitterDesc, playfieldHalfExtent);
    UpdateLayout();
}

void KatamariParticleSettingsPanel::SetBounds(const RectF &bounds) noexcept
{
    m_bounds = bounds;
    UpdateLayout();
}

KatamariParticleUiAction KatamariParticleSettingsPanel::Update(const MouseState &mouse)
{
    if (!m_isOpen)
    {
        return KatamariParticleUiAction::None;
    }

    bool sliderMoved = false;

    m_spawnSection.Update(mouse);
    m_drawSection.Update(mouse);

    UpdateLayout();

    if (m_spawnSection.IsExpanded())
    {
        {
            const float before = m_spawnRateSlider.GetValue();
            m_spawnRateSlider.Update(mouse);
            sliderMoved = sliderMoved || DidSliderValueChange(before, m_spawnRateSlider.GetValue());
        }

        {
            const float before = m_spawnVolumeScaleSlider.GetValue();
            m_spawnVolumeScaleSlider.Update(mouse);
            sliderMoved = sliderMoved || DidSliderValueChange(before, m_spawnVolumeScaleSlider.GetValue());
        }

        {
            const float before = m_spawnCenterHeightSlider.GetValue();
            m_spawnCenterHeightSlider.Update(mouse);
            sliderMoved = sliderMoved || DidSliderValueChange(before, m_spawnCenterHeightSlider.GetValue());
        }

        {
            const float before = m_spawnHalfHeightSlider.GetValue();
            m_spawnHalfHeightSlider.Update(mouse);
            sliderMoved = sliderMoved || DidSliderValueChange(before, m_spawnHalfHeightSlider.GetValue());
        }
    }

    if (m_drawSection.IsExpanded())
    {
        {
            const float before = m_lifetimeSlider.GetValue();
            m_lifetimeSlider.Update(mouse);
            sliderMoved = sliderMoved || DidSliderValueChange(before, m_lifetimeSlider.GetValue());
        }

        {
            const float before = m_startSizeSlider.GetValue();
            m_startSizeSlider.Update(mouse);
            sliderMoved = sliderMoved || DidSliderValueChange(before, m_startSizeSlider.GetValue());
        }

        {
            const float before = m_drawAlphaScaleSlider.GetValue();
            m_drawAlphaScaleSlider.Update(mouse);
            sliderMoved = sliderMoved || DidSliderValueChange(before, m_drawAlphaScaleSlider.GetValue());
        }

        {
            const float before = m_drawSizeScaleSlider.GetValue();
            m_drawSizeScaleSlider.Update(mouse);
            sliderMoved = sliderMoved || DidSliderValueChange(before, m_drawSizeScaleSlider.GetValue());
        }
    }

    return sliderMoved ? KatamariParticleUiAction::SliderMoved
                       : KatamariParticleUiAction::None;
}

void KatamariParticleSettingsPanel::Render(ShapeRenderer2D &shapeRenderer) const
{
    if (!m_isOpen)
    {
        return;
    }

    shapeRenderer.DrawFilledRect(m_bounds.X, m_bounds.Y, m_bounds.Width, m_bounds.Height, m_theme.PanelBorder);
    shapeRenderer.DrawFilledRect(
        m_bounds.X + 2.0f,
        m_bounds.Y + 2.0f,
        m_bounds.Width - 4.0f,
        m_bounds.Height - 4.0f,
        m_theme.PanelBackground
    );

    constexpr float titleScale = 0.56f;
    constexpr float hintScale = 0.42f;

    const float titleX = m_bounds.X + m_theme.PanelPadding;
    const float titleY = m_bounds.Y + 8.0f;

    BitmapFont::DrawString(shapeRenderer, titleX, titleY, "KATAMARI PARTICLES", m_theme.HeaderText, titleScale);
    BitmapFont::DrawString(
        shapeRenderer,
        titleX,
        titleY + 20.0f,
        "GPU EMITTER",
        Color(0.62f, 0.72f, 0.86f, 1.0f),
        hintScale
    );

    m_spawnSection.Render(shapeRenderer);
    m_drawSection.Render(shapeRenderer);

    if (m_spawnSection.IsExpanded())
    {
        m_spawnRateSlider.Render(shapeRenderer);
        m_spawnVolumeScaleSlider.Render(shapeRenderer);
        m_spawnCenterHeightSlider.Render(shapeRenderer);
        m_spawnHalfHeightSlider.Render(shapeRenderer);
    }

    if (m_drawSection.IsExpanded())
    {
        m_lifetimeSlider.Render(shapeRenderer);
        m_startSizeSlider.Render(shapeRenderer);
        m_drawAlphaScaleSlider.Render(shapeRenderer);
        m_drawSizeScaleSlider.Render(shapeRenderer);
    }
}

void KatamariParticleSettingsPanel::ApplyToEmitterDesc(
    GpuParticleEmitterDesc &emitterDesc,
    const float playfieldHalfExtent
) const noexcept
{
    const float volumeScale = m_spawnVolumeScaleSlider.GetValue();

    emitterDesc.SpawnRateParticlesPerSecond = m_spawnRateSlider.GetValue();
    emitterDesc.SpawnVolumeCenter = DirectX::SimpleMath::Vector3(
        0.0f,
        m_spawnCenterHeightSlider.GetValue(),
        0.0f
    );
    emitterDesc.SpawnVolumeHalfExtents = DirectX::SimpleMath::Vector3(
        playfieldHalfExtent * volumeScale,
        m_spawnHalfHeightSlider.GetValue(),
        playfieldHalfExtent * volumeScale
    );
    emitterDesc.LifetimeSeconds = m_lifetimeSlider.GetValue();
    emitterDesc.StartSize = m_startSizeSlider.GetValue();
    emitterDesc.DrawAlphaScale = m_drawAlphaScaleSlider.GetValue();
    emitterDesc.DrawSizeScale = m_drawSizeScaleSlider.GetValue();
}

void KatamariParticleSettingsPanel::SetValuesFromEmitterDesc(
    const GpuParticleEmitterDesc &emitterDesc,
    const float playfieldHalfExtent
) noexcept
{
    m_spawnRateSlider.SetValue(emitterDesc.SpawnRateParticlesPerSecond);
    m_spawnVolumeScaleSlider.SetValue(ResolveVolumeScale(emitterDesc, playfieldHalfExtent));
    m_spawnCenterHeightSlider.SetValue(emitterDesc.SpawnVolumeCenter.y);
    m_spawnHalfHeightSlider.SetValue(emitterDesc.SpawnVolumeHalfExtents.y);
    m_lifetimeSlider.SetValue(emitterDesc.LifetimeSeconds);
    m_startSizeSlider.SetValue(emitterDesc.StartSize);
    m_drawAlphaScaleSlider.SetValue(emitterDesc.DrawAlphaScale);
    m_drawSizeScaleSlider.SetValue(emitterDesc.DrawSizeScale);
}

bool KatamariParticleSettingsPanel::IsOpen() const noexcept
{
    return m_isOpen;
}

bool KatamariParticleSettingsPanel::IsInteracting() const noexcept
{
    return m_spawnRateSlider.IsDragging()
        || m_spawnVolumeScaleSlider.IsDragging()
        || m_spawnCenterHeightSlider.IsDragging()
        || m_spawnHalfHeightSlider.IsDragging()
        || m_lifetimeSlider.IsDragging()
        || m_startSizeSlider.IsDragging()
        || m_drawAlphaScaleSlider.IsDragging()
        || m_drawSizeScaleSlider.IsDragging();
}

void KatamariParticleSettingsPanel::Toggle() noexcept
{
    m_isOpen = !m_isOpen;
}

void KatamariParticleSettingsPanel::UpdateLayout() noexcept
{
    m_bounds.Height = CalculateContentHeight();

    const float x = m_bounds.X + m_theme.PanelPadding;
    float y = m_bounds.Y + 54.0f;
    const float width = m_bounds.Width - m_theme.PanelPadding * 2.0f;

    m_spawnSection.SetBounds(RectF{x, y, width, m_theme.HeaderHeight});
    y += m_theme.HeaderHeight + m_theme.WidgetSpacing;

    if (m_spawnSection.IsExpanded())
    {
        m_spawnRateSlider.SetBounds(RectF{x, y, width, m_theme.SliderHeight});
        y += m_theme.SliderHeight + m_theme.WidgetSpacing;

        m_spawnVolumeScaleSlider.SetBounds(RectF{x, y, width, m_theme.SliderHeight});
        y += m_theme.SliderHeight + m_theme.WidgetSpacing;

        m_spawnCenterHeightSlider.SetBounds(RectF{x, y, width, m_theme.SliderHeight});
        y += m_theme.SliderHeight + m_theme.WidgetSpacing;

        m_spawnHalfHeightSlider.SetBounds(RectF{x, y, width, m_theme.SliderHeight});
        y += m_theme.SliderHeight + m_theme.SectionSpacing;
    }

    m_drawSection.SetBounds(RectF{x, y, width, m_theme.HeaderHeight});
    y += m_theme.HeaderHeight + m_theme.WidgetSpacing;

    if (m_drawSection.IsExpanded())
    {
        m_lifetimeSlider.SetBounds(RectF{x, y, width, m_theme.SliderHeight});
        y += m_theme.SliderHeight + m_theme.WidgetSpacing;

        m_startSizeSlider.SetBounds(RectF{x, y, width, m_theme.SliderHeight});
        y += m_theme.SliderHeight + m_theme.WidgetSpacing;

        m_drawAlphaScaleSlider.SetBounds(RectF{x, y, width, m_theme.SliderHeight});
        y += m_theme.SliderHeight + m_theme.WidgetSpacing;

        m_drawSizeScaleSlider.SetBounds(RectF{x, y, width, m_theme.SliderHeight});
    }
}

float KatamariParticleSettingsPanel::CalculateContentHeight() const noexcept
{
    float height = 54.0f + m_theme.PanelPadding;

    height += m_theme.HeaderHeight + m_theme.WidgetSpacing;
    if (m_spawnSection.IsExpanded())
    {
        height += (m_theme.SliderHeight * 4.0f) + (m_theme.WidgetSpacing * 3.0f) + m_theme.SectionSpacing;
    }

    height += m_theme.HeaderHeight + m_theme.WidgetSpacing;
    if (m_drawSection.IsExpanded())
    {
        height += (m_theme.SliderHeight * 4.0f) + (m_theme.WidgetSpacing * 3.0f) + m_theme.SectionSpacing;
    }

    return height;
}
