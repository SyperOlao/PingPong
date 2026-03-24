//
// Created by SyperOlao on 25.03.2026.
//

#include "SolarSystemSettingsPanel.h"

#include "Core/Graphics2D/ShapeRenderer2D.h"

void SolarSystemSettingsPanel::Initialize()
{
    m_dynamicsSection.SetTheme(&m_theme);
    m_dynamicsSection.SetTitle("Dynamics");
    m_dynamicsSection.SetExpanded(true);

    m_orbitSection.SetTheme(&m_theme);
    m_orbitSection.SetTitle("Orbit Shape");
    m_orbitSection.SetExpanded(true);

    m_planetRotationSlider.SetTheme(&m_theme);
    m_planetRotationSlider.SetLabel("Planet Rotation");
    m_planetRotationSlider.SetRange(0.2f, 4.0f);
    m_planetRotationSlider.SetValue(1.0f);

    m_moonRotationSlider.SetTheme(&m_theme);
    m_moonRotationSlider.SetLabel("Moon Rotation");
    m_moonRotationSlider.SetRange(0.2f, 5.0f);
    m_moonRotationSlider.SetValue(1.0f);

    m_planetOrbitSlider.SetTheme(&m_theme);
    m_planetOrbitSlider.SetLabel("Planet Orbit Radius");
    m_planetOrbitSlider.SetRange(0.5f, 2.5f);
    m_planetOrbitSlider.SetValue(1.0f);

    m_moonOrbitSlider.SetTheme(&m_theme);
    m_moonOrbitSlider.SetLabel("Moon Orbit Radius");
    m_moonOrbitSlider.SetRange(0.5f, 2.5f);
    m_moonOrbitSlider.SetValue(1.0f);

    m_eccentricitySlider.SetTheme(&m_theme);
    m_eccentricitySlider.SetLabel("Orbit Eccentricity");
    m_eccentricitySlider.SetRange(0.2f, 2.0f);
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
    }

    if (m_orbitSection.IsExpanded())
    {
        m_planetOrbitSlider.Update(mouse);
        m_moonOrbitSlider.Update(mouse);
        m_eccentricitySlider.Update(mouse);
    }
}

void SolarSystemSettingsPanel::Render(ShapeRenderer2D& shapeRenderer) const
{
    if (!m_isOpen)
    {
        return;
    }

    shapeRenderer.DrawFilledRect(m_bounds.X, m_bounds.Y, m_bounds.Width, m_bounds.Height, m_theme.PanelBackground);

    m_dynamicsSection.Render(shapeRenderer);
    m_orbitSection.Render(shapeRenderer);

    if (m_dynamicsSection.IsExpanded())
    {
        m_planetRotationSlider.Render(shapeRenderer);
        m_moonRotationSlider.Render(shapeRenderer);
    }

    if (m_orbitSection.IsExpanded())
    {
        m_planetOrbitSlider.Render(shapeRenderer);
        m_moonOrbitSlider.Render(shapeRenderer);
        m_eccentricitySlider.Render(shapeRenderer);
    }
}

SolarSystemTuning SolarSystemSettingsPanel::GetTuning() const noexcept
{
    SolarSystemTuning tuning{};
    tuning.PlanetRotationScale = m_planetRotationSlider.GetValue();
    tuning.MoonRotationScale = m_moonRotationSlider.GetValue();
    tuning.PlanetOrbitRadiusScale = m_planetOrbitSlider.GetValue();
    tuning.MoonOrbitRadiusScale = m_moonOrbitSlider.GetValue();
    tuning.OrbitEccentricityScale = m_eccentricitySlider.GetValue();
    return tuning;
}

bool SolarSystemSettingsPanel::IsOpen() const noexcept
{
    return m_isOpen;
}

void SolarSystemSettingsPanel::Toggle() noexcept
{
    m_isOpen = !m_isOpen;
}

void SolarSystemSettingsPanel::UpdateLayout() noexcept
{
    const float x = m_bounds.X + m_theme.PanelPadding;
    float y = m_bounds.Y + m_theme.PanelPadding;
    const float width = m_bounds.Width - m_theme.PanelPadding * 2.0f;

    m_dynamicsSection.SetBounds(RectF{x, y, width, m_theme.HeaderHeight});
    y += m_theme.HeaderHeight + m_theme.WidgetSpacing;

    if (m_dynamicsSection.IsExpanded())
    {
        m_planetRotationSlider.SetBounds(RectF{x, y, width, m_theme.SliderHeight});
        y += m_theme.SliderHeight + m_theme.WidgetSpacing;

        m_moonRotationSlider.SetBounds(RectF{x, y, width, m_theme.SliderHeight});
        y += m_theme.SliderHeight + m_theme.SectionSpacing;
    }

    m_orbitSection.SetBounds(RectF{x, y, width, m_theme.HeaderHeight});
    y += m_theme.HeaderHeight + m_theme.WidgetSpacing;

    if (m_orbitSection.IsExpanded())
    {
        m_planetOrbitSlider.SetBounds(RectF{x, y, width, m_theme.SliderHeight});
        y += m_theme.SliderHeight + m_theme.WidgetSpacing;

        m_moonOrbitSlider.SetBounds(RectF{x, y, width, m_theme.SliderHeight});
        y += m_theme.SliderHeight + m_theme.WidgetSpacing;

        m_eccentricitySlider.SetBounds(RectF{x, y, width, m_theme.SliderHeight});
        y += m_theme.SliderHeight + m_theme.SectionSpacing;
    }
}