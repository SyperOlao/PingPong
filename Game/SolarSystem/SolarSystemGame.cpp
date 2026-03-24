//
// Created by SyperOlao on 19.03.2026.
//
#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "SolarSystemGame.h"


#include <algorithm>
#include <cmath>
#include <format>
#include <string>

#include "Core/App/AppContext.h"
#include "Core/Graphics/Camera.h"
#include "Core/Graphics/Color.h"
#include "Core/Graphics/GraphicsDevice.h"
#include "Core/Input/InputSystem.h"
#include "Core/Input/Keyboard.h"
#include "Core/Input/RawInputHandler.h"
#include "Core/Platform/Window.h"
#include "Core/UI/BitmapFont.h"
#include "Game/SolarSystem/Entities/OrbitalBody.h"

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

namespace
{
    constexpr float kMoveSpeed = 18.0f;
    constexpr float kRotationSpeed = 1.8f;

    constexpr float kMouseFpsSensitivity = 0.0035f;
    constexpr float kMouseOrbitSensitivity = 0.0040f;

    constexpr float kOrbitZoomSpeed = 25.0f;
    constexpr float kOrbitMinRadius = 8.0f;
    constexpr float kOrbitMaxRadius = 240.0f;

    constexpr float kOrbitMinPitch = -1.35f;
    constexpr float kOrbitMaxPitch = 1.35f;

    POINT GetMouseClientPosition(HWND__* const hwnd)
    {
        POINT point{};
        GetCursorPos(&point);
        ScreenToClient(hwnd, &point);
        return point;
    }
}

void SolarSystemGame::Initialize(AppContext& context)
{
    if (!context.IsValid())
    {
        throw std::runtime_error("SolarSystemGame::Initialize received invalid AppContext.");
    }

    m_scene.Initialize();

    m_renderer3D.Initialize(*context.Graphics);

    m_fpsCamera.SetPosition(Vector3(0.0f, 8.0f, -30.0f));
    m_fpsCamera.SetRotation(0.0f, 0.15f);

    m_settingsPanel.Initialize();
    m_settingsPanel.SetBounds(RectF{20.0f, 170.0f, 360.0f, 470.0f});

    m_orbitCamera.SetTarget(Vector3(0.0f, 0.0f, 0.0f));
    m_orbitCamera.SetYawPitchRadius(m_orbitCameraYaw, m_orbitCameraPitch, m_orbitCameraRadius);

    SetCameraMode(CameraMode::Orbit);

    m_fpsAccumulator = 0.0f;
    m_fpsFrames = 0;
    m_displayFps = 0;

    m_initialized = true;
}

void SolarSystemGame::Update(AppContext& context, const float deltaTime)
{
    if (!m_initialized)
    {
        return;
    }

    UpdateFpsCounter(deltaTime);
    HandleGlobalInput(context);

    const auto& raw = RawInputHandler::Instance();

    HWND__* const hwnd = context.MainWindow->GetHandle();
    const auto [x, y] = GetMouseClientPosition(hwnd);

    MouseState mouse{};
    mouse.X = static_cast<float>(x);
    mouse.Y = static_cast<float>(y);
    mouse.LeftDown = raw.IsLeftMouseDown();
    mouse.LeftPressed = raw.IsLeftMouseDown() && !m_prevLeftMouseDown;
    mouse.LeftReleased = !raw.IsLeftMouseDown() && m_prevLeftMouseDown;

    m_settingsPanel.Update(mouse);
    m_prevLeftMouseDown = raw.IsLeftMouseDown();

    const SolarSystemTuning tuning = m_settingsPanel.GetTuning();
    m_scene.SetTuning(tuning);

    if (const bool uiInteracting = m_settingsPanel.IsInteracting(); !uiInteracting)
    {
        switch (m_cameraMode)
        {
            case CameraMode::Fps:
                UpdateFpsCamera(context, deltaTime);
                break;

            case CameraMode::Orbit:
                UpdateOrbitCamera(context, deltaTime);
                break;

            default:
                break;
        }
    }

    m_scene.Update(deltaTime);

    RawInputHandler::Instance().ClearFrameDeltas();
}

void SolarSystemGame::Render(AppContext& context)
{
    if (!m_initialized)
    {
        return;
    }

    const int width = context.Graphics->GetWidth();
    const int height = context.Graphics->GetHeight();

    if (width <= 0 || height <= 0)
    {
        return;
    }

    const float aspectRatio = static_cast<float>(width) / static_cast<float>(height);

    m_renderer3D.BeginFrame3D();

    const Camera& activeCamera = GetActiveCamera();
    const Matrix view = activeCamera.GetViewMatrix();
    const Matrix projection = activeCamera.GetProjectionMatrix(aspectRatio);

    for (const auto& root : m_scene.GetRoots())
    {
        RenderBodyRecursive(*root, view, projection);
    }

    if (context.Font != nullptr)
    {
        const std::string cameraLabel = (m_cameraMode == CameraMode::Fps) ? "FPS" : "ORBIT";
        const std::string projectionLabel =
            (activeCamera.GetProjectionMode() == ProjectionMode::PerspectiveFov)
                ? "FOV"
                : "OFFCENTER";

        constexpr float hudScale = 0.54f;
        constexpr float helpScale = 0.40f;

        const float x = 16.0f;
        float y = 16.0f;

        BitmapFont::DrawString(
            *context.Shape2D,
            x,
            y,
            std::format("FPS: {}", m_displayFps),
            Color(1, 1, 1, 1),
            hudScale
        );
        y += 18.0f;

        BitmapFont::DrawString(
            *context.Shape2D,
            x,
            y,
            std::format("CAMERA: {}", cameraLabel),
            Color(1, 1, 1, 1),
            hudScale
        );
        y += 18.0f;

        BitmapFont::DrawString(
            *context.Shape2D,
            x,
            y,
            std::format("PROJECTION: {}", projectionLabel),
            Color(1, 1, 1, 1),
            hudScale
        );
        y += 26.0f;

        BitmapFont::DrawString(
            *context.Shape2D,
            x,
            y,
            "F1 FPS   F2 ORBIT   P PROJECTION   TAB SETTINGS",
            Color(0.86f, 0.90f, 1.0f, 1.0f),
            helpScale
        );
        y += 15.0f;

        BitmapFont::DrawString(
            *context.Shape2D,
            x,
            y,
            "FPS: WASD MOVE   ARROWS LOOK   RMB LOOK",
            Color(0.70f, 0.78f, 0.90f, 1.0f),
            helpScale
        );
        y += 15.0f;

        BitmapFont::DrawString(
            *context.Shape2D,
            x,
            y,
            "ORBIT: RMB ROTATE   W/S ZOOM   ARROWS TILT",
            Color(0.70f, 0.78f, 0.90f, 1.0f),
            helpScale
        );
    }

    m_settingsPanel.Render(*context.Shape2D);
}

void SolarSystemGame::Shutdown(AppContext& context)
{
    (void)context;
    m_initialized = false;
}

void SolarSystemGame::UpdateFpsCounter(const float deltaTime) noexcept
{
    m_fpsAccumulator += deltaTime;
    ++m_fpsFrames;

    if (m_fpsAccumulator >= 0.25f)
    {
        m_displayFps = static_cast<int>(std::round(static_cast<float>(m_fpsFrames) / m_fpsAccumulator));
        m_fpsAccumulator = 0.0f;
        m_fpsFrames = 0;
    }
}

void SolarSystemGame::HandleGlobalInput(const AppContext& context)
{
    const Keyboard& keyboard = context.Input->GetKeyboard();

    if (keyboard.WasVirtualKeyPressed(VK_F1))
    {
        SetCameraMode(CameraMode::Fps);
    }

    if (keyboard.WasVirtualKeyPressed(VK_F2))
    {
        SetCameraMode(CameraMode::Orbit);
    }

    if (keyboard.WasVirtualKeyPressed('P'))
    {
        if (Camera& camera = GetActiveCamera(); camera.GetProjectionMode() == ProjectionMode::PerspectiveFov)
        {
            camera.SetProjectionMode(ProjectionMode::PerspectiveOffCenter);
        }
        else
        {
            camera.SetProjectionMode(ProjectionMode::PerspectiveFov);
        }
    }

    if (keyboard.WasKeyPressed(Key::Escape))
    {
        PostQuitMessage(0);
    }

    if (keyboard.WasKeyPressed(Key::Tab))
    {
        m_settingsPanel.Toggle();
    }
}

void SolarSystemGame::UpdateFpsCamera(const AppContext& context, const float deltaTime)
{
    const Keyboard& keyboard = context.Input->GetKeyboard();

    if (keyboard.IsKeyDown(Key::W)) m_fpsCamera.MoveForward(kMoveSpeed * deltaTime);
    if (keyboard.IsKeyDown(Key::S)) m_fpsCamera.MoveForward(-kMoveSpeed * deltaTime);
    if (keyboard.IsKeyDown(Key::D)) m_fpsCamera.MoveRight(kMoveSpeed * deltaTime);
    if (keyboard.IsKeyDown(Key::A)) m_fpsCamera.MoveRight(-kMoveSpeed * deltaTime);

    float yawDelta = 0.0f;
    float pitchDelta = 0.0f;

    if (const auto& raw = RawInputHandler::Instance(); raw.IsRightMouseDown())
    {
        yawDelta += static_cast<float>(raw.GetMouseDeltaX()) * kMouseFpsSensitivity;
        pitchDelta -= static_cast<float>(raw.GetMouseDeltaY()) * kMouseFpsSensitivity;
    }

    if (keyboard.IsKeyDown(Key::Left)) yawDelta -= kRotationSpeed * deltaTime;
    if (keyboard.IsKeyDown(Key::Right)) yawDelta += kRotationSpeed * deltaTime;
    if (keyboard.IsKeyDown(Key::Up)) pitchDelta += kRotationSpeed * deltaTime;
    if (keyboard.IsKeyDown(Key::Down)) pitchDelta -= kRotationSpeed * deltaTime;

    m_fpsCamera.AddRotation(yawDelta, pitchDelta);
}

void SolarSystemGame::UpdateOrbitCamera(const AppContext& context, const float deltaTime)
{
    const Keyboard& keyboard = context.Input->GetKeyboard();

    if (const auto& raw = RawInputHandler::Instance(); raw.IsRightMouseDown())
    {
        m_orbitCameraYaw += static_cast<float>(raw.GetMouseDeltaX()) * kMouseOrbitSensitivity;
        m_orbitCameraPitch -= static_cast<float>(raw.GetMouseDeltaY()) * kMouseOrbitSensitivity;
    }

    if (keyboard.IsKeyDown(Key::Left)) m_orbitCameraYaw -= 1.5f * deltaTime;
    if (keyboard.IsKeyDown(Key::Right)) m_orbitCameraYaw += 1.5f * deltaTime;
    if (keyboard.IsKeyDown(Key::Up)) m_orbitCameraPitch += 1.1f * deltaTime;
    if (keyboard.IsKeyDown(Key::Down)) m_orbitCameraPitch -= 1.1f * deltaTime;

    if (keyboard.IsKeyDown(Key::W)) m_orbitCameraRadius -= kOrbitZoomSpeed * deltaTime;
    if (keyboard.IsKeyDown(Key::S)) m_orbitCameraRadius += kOrbitZoomSpeed * deltaTime;

    m_orbitCameraPitch = std::clamp(m_orbitCameraPitch, kOrbitMinPitch, kOrbitMaxPitch);
    m_orbitCameraRadius = std::clamp(m_orbitCameraRadius, kOrbitMinRadius, kOrbitMaxRadius);

    m_orbitCamera.SetYawPitchRadius(
        m_orbitCameraYaw,
        m_orbitCameraPitch,
        m_orbitCameraRadius
    );
}

void SolarSystemGame::SetCameraMode(const CameraMode mode) noexcept
{
    m_cameraMode = mode;
}

Camera& SolarSystemGame::GetActiveCamera() noexcept
{
    return (m_cameraMode == CameraMode::Fps)
        ? static_cast<Camera&>(m_fpsCamera)
        : static_cast<Camera&>(m_orbitCamera);
}

const Camera& SolarSystemGame::GetActiveCamera() const noexcept
{
    return (m_cameraMode == CameraMode::Fps)
        ? static_cast<const Camera&>(m_fpsCamera)
        : static_cast<const Camera&>(m_orbitCamera);
}

void SolarSystemGame::RenderBodyRecursive(
    const OrbitalBody& body,
    const Matrix& view,
    const Matrix& projection
)
{
    switch (body.MeshType)
    {
        case BodyMeshType::Sphere:
            m_renderer3D.DrawSphere(body.GetWorldMatrix(), view, projection, body.BaseColor);
            break;

        case BodyMeshType::Box:
            m_renderer3D.DrawBox(body.GetWorldMatrix(), view, projection, body.BaseColor);
            break;
    }

    for (const auto& child : body.GetChildren())
    {
        RenderBodyRecursive(*child, view, projection);
    }
}