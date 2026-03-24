//
// Created by SyperOlao on 25.03.2026.
//

#include "SolarSystemSettingsPanel.h"

#include "Core/Graphics2D/ShapeRenderer2D.h"
#include "Core/UI/BitmapFont.h"

void SolarSystemSettingsPanel::Initialize()
{
    m_dynamicsSection.SetTheme(&m_theme);
    m_dynamicsSection.SetTitle("Dynamics");
    m_dynamicsSection.SetExpanded(true);

    m_orbitSection.SetTheme(&m_theme);
    m_orbitSection.SetTitle("Orbit");
    m_orbitSection.SetExpanded(true);

    m_planetRotationSlider.SetTheme(&m_theme);
    m_planetRotationSlider.SetLabel("Planet spin");
    m_planetRotationSlider.SetRange(0.2f, 4.0f);
    m_planetRotationSlider.SetValue(1.0f);

    m_moonRotationSlider.SetTheme(&m_theme);
    m_moonRotationSlider.SetLabel("Moon spin");
    m_moonRotationSlider.SetRange(0.2f, 5.0f);
    m_moonRotationSlider.SetValue(1.0f);

    m_planetOrbitSpeedSlider.SetTheme(&m_theme);
    m_planetOrbitSpeedSlider.SetLabel("Planet orbit speed");
    m_planetOrbitSpeedSlider.SetRange(0.2f, 4.0f);
    m_planetOrbitSpeedSlider.SetValue(1.0f);

    m_moonOrbitSpeedSlider.SetTheme(&m_theme);
    m_moonOrbitSpeedSlider.SetLabel("Moon orbit speed");
    m_moonOrbitSpeedSlider.SetRange(0.2f, 5.0f);
    m_moonOrbitSpeedSlider.SetValue(1.0f);

    m_orbitRadiusSlider.SetTheme(&m_theme);
    m_orbitRadiusSlider.SetLabel("Orbit size");
    m_orbitRadiusSlider.SetRange(0.5f, 2.5f);
    m_orbitRadiusSlider.SetValue(1.0f);

    m_eccentricitySlider.SetTheme(&m_theme);
    m_eccentricitySlider.SetLabel("Orbit eccentricity");
    m_eccentricitySlider.SetRange(0.3f, 2.0f);
    m_eccentricitySlider.SetValue(1.0f);

    UpdateLayout();
}

void SolarSystemSettingsPanel::SetBounds(const RectF& bounds) noexcept
{
    m_bounds = bounds;
    UpdateLayout();
}

void SolarSystemSettingsPanel::Update(const MouseState& mouse)
{
    if (!m_isOpen)
    {
        return;
    }

    m_dynamicsSection.Update(mouse);
    m_orbitSection.Update(mouse);

    UpdateLayout();

    if (m_dynamicsSection.IsExpanded())
    {
        m_planetRotationSlider.Update(mouse);
        m_moonRotationSlider.Update(mouse);
        m_planetOrbitSpeedSlider.Update(mouse);
        m_moonOrbitSpeedSlider.Update(mouse);
    }

    if (m_orbitSection.IsExpanded())
    {
        m_orbitRadiusSlider.Update(mouse);
        m_eccentricitySlider.Update(mouse);
    }
}

void SolarSystemSettingsPanel::Render(ShapeRenderer2D& shapeRenderer) const
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

    BitmapFont::DrawString(shapeRenderer, titleX, titleY, "SOLAR SYSTEM SETTINGS", m_theme.HeaderText, titleScale);
    BitmapFont::DrawString(
        shapeRenderer,
        titleX,
        titleY + 20.0f,
        "TAB - HIDE  |  DRAG SLIDER TO APPLY",
        Color(0.62f, 0.72f, 0.86f, 1.0f),
        hintScale
    );

    m_dynamicsSection.Render(shapeRenderer);
    m_orbitSection.Render(shapeRenderer);

    if (m_dynamicsSection.IsExpanded())
    {
        m_planetRotationSlider.Render(shapeRenderer);
        m_moonRotationSlider.Render(shapeRenderer);
        m_planetOrbitSpeedSlider.Render(shapeRenderer);
        m_moonOrbitSpeedSlider.Render(shapeRenderer);
    }

    if (m_orbitSection.IsExpanded())
    {
        m_orbitRadiusSlider.Render(shapeRenderer);
        m_eccentricitySlider.Render(shapeRenderer);
    }
}

SolarSystemTuning SolarSystemSettingsPanel::GetTuning() const noexcept
{
    SolarSystemTuning tuning{};
    tuning.PlanetRotationScale = m_planetRotationSlider.GetValue();
    tuning.MoonRotationScale = m_moonRotationSlider.GetValue();
    tuning.PlanetOrbitSpeedScale = m_planetOrbitSpeedSlider.GetValue();
    tuning.MoonOrbitSpeedScale = m_moonOrbitSpeedSlider.GetValue();
    tuning.OrbitRadiusScale = m_orbitRadiusSlider.GetValue();
    tuning.OrbitEccentricityScale = m_eccentricitySlider.GetValue();
    return tuning;
}

bool SolarSystemSettingsPanel::IsOpen() const noexcept
{
    return m_isOpen;
}

bool SolarSystemSettingsPanel::IsInteracting() const noexcept
{
    return m_planetRotationSlider.IsDragging() ||
           m_moonRotationSlider.IsDragging() ||
           m_planetOrbitSpeedSlider.IsDragging() ||
           m_moonOrbitSpeedSlider.IsDragging() ||
           m_orbitRadiusSlider.IsDragging() ||
           m_eccentricitySlider.IsDragging();
}

void SolarSystemSettingsPanel::Toggle() noexcept
{
    m_isOpen = !m_isOpen;
}

void SolarSystemSettingsPanel::UpdateLayout() noexcept
{
    m_bounds.Height = CalculateContentHeight();

    const float x = m_bounds.X + m_theme.PanelPadding;
    float y = m_bounds.Y + 54.0f;
    const float width = m_bounds.Width - m_theme.PanelPadding * 2.0f;

    m_dynamicsSection.SetBounds(RectF{x, y, width, m_theme.HeaderHeight});
    y += m_theme.HeaderHeight + m_theme.WidgetSpacing;

    if (m_dynamicsSection.IsExpanded())
    {
        m_planetRotationSlider.SetBounds(RectF{x, y, width, m_theme.SliderHeight});
        y += m_theme.SliderHeight + m_theme.WidgetSpacing;

        m_moonRotationSlider.SetBounds(RectF{x, y, width, m_theme.SliderHeight});
        y += m_theme.SliderHeight + m_theme.WidgetSpacing;

        m_planetOrbitSpeedSlider.SetBounds(RectF{x, y, width, m_theme.SliderHeight});
        y += m_theme.SliderHeight + m_theme.WidgetSpacing;

        m_moonOrbitSpeedSlider.SetBounds(RectF{x, y, width, m_theme.SliderHeight});
        y += m_theme.SliderHeight + m_theme.SectionSpacing;
    }

    m_orbitSection.SetBounds(RectF{x, y, width, m_theme.HeaderHeight});
    y += m_theme.HeaderHeight + m_theme.WidgetSpacing;

    if (m_orbitSection.IsExpanded())
    {
        m_orbitRadiusSlider.SetBounds(RectF{x, y, width, m_theme.SliderHeight});
        y += m_theme.SliderHeight + m_theme.WidgetSpacing;

        m_eccentricitySlider.SetBounds(RectF{x, y, width, m_theme.SliderHeight});
    }
}

float SolarSystemSettingsPanel::CalculateContentHeight() const noexcept
{
    float height = 54.0f + m_theme.PanelPadding;

    height += m_theme.HeaderHeight + m_theme.WidgetSpacing;
    if (m_dynamicsSection.IsExpanded())
    {
        height += (m_theme.SliderHeight * 4.0f) + (m_theme.WidgetSpacing * 3.0f) + m_theme.SectionSpacing;
    }

    height += m_theme.HeaderHeight + m_theme.WidgetSpacing;
    if (m_orbitSection.IsExpanded())
    {
        height += (m_theme.SliderHeight * 2.0f) + m_theme.WidgetSpacing + m_theme.SectionSpacing;
    }

    return height;
}