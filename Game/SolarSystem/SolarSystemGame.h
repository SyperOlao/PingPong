//
// Created by SyperOlao on 19.03.2026.
//

#ifndef PINGPONG_SOLARSYSTEMGAME_H
#define PINGPONG_SOLARSYSTEMGAME_H
#include "SolarSystemScene.h"
#include "Core/App/IGame.h"
#include "Core/Graphics/FpsCamera.h"
#include "../../Core/Graphics/OrbitCamera.h"
#include "Core/Graphics/PrimitiveRenderer3D.h"
#include "Rendering/SpaceBackdropRenderer.h"
#include "UI/SolarSystemSettingsPanel.h"

struct AppContext;
class Camera;
class OrbitalBody;
#include "Systems/SolarSystemEngineAudioController.h"
enum class CameraMode : std::uint8_t
{
    Fps = 0,
    Orbit
};


class SolarSystemGame final : public IGame
{
public:
    void Initialize(AppContext& context) override;
    void Update(AppContext& context, float deltaTime) override;
    void Render(AppContext& context) override;
    void Shutdown(AppContext& context) override;

private:
    void UpdateFpsCounter(float deltaTime) noexcept;

    void HandleGlobalInput(const AppContext& context);
    void UpdateFpsCamera(const AppContext& context, float deltaTime);
    void UpdateOrbitCamera(const AppContext& context, float deltaTime);

    void InitializeEngineAudio(const AppContext& context);
    void UpdateEngineAudio(const AppContext& context, float deltaTime);
    void ShutdownEngineAudio(const AppContext& context) noexcept;
    [[nodiscard]] bool IsEngineMovementInputActive(const AppContext& context) const noexcept;

    void SetCameraMode(CameraMode mode) noexcept;
    [[nodiscard]] Camera& GetActiveCamera() noexcept;
    [[nodiscard]] const Camera& GetActiveCamera() const noexcept;

    void RenderOrbitRecursive(
        const OrbitalBody& body,
        const DirectX::SimpleMath::Matrix& parentWorld,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& projection
    );

    void RenderBodyRecursive(
        const OrbitalBody& body,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& projection
    );

private:
    SolarSystemScene m_scene{};

    PrimitiveRenderer3D m_renderer3D{};
    SpaceBackdropRenderer m_spaceBackdrop{};

    FpsCamera m_fpsCamera{};
    OrbitCamera m_orbitCamera{};
    CameraMode m_cameraMode{CameraMode::Orbit};
    SolarSystemSettingsPanel m_settingsPanel{};

    bool m_prevLeftMouseDown{false};
    float m_orbitCameraYaw{0.6f};
    float m_orbitCameraPitch{0.5f};
    float m_orbitCameraRadius{45.0f};

    float m_fpsAccumulator{0.0f};
    int m_fpsFrames{0};
    int m_displayFps{0};
    SolarSystemEngineAudioController m_engineAudio{};


    bool m_initialized{false};
};
#endif //PINGPONG_SOLARSYSTEMGAME_H