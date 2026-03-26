#include "KatamariCameraController.h"

#include "Core/Graphics/FollowCamera.h"

#include <algorithm>
#include <cmath>

using DirectX::SimpleMath::Vector3;

void KatamariCameraController::Initialize(FollowCamera &followCamera) noexcept
{
    m_followCamera = &followCamera;
    m_cameraYawRadians = 0.0f;
    m_cameraPitchRadians = -0.08f;
    m_hasInitializedYaw = false;
    m_previousMouseDeltaX = 0.0f;
    m_previousMouseDeltaY = 0.0f;
    m_hasPreviousMouseSample = false;
}

void KatamariCameraController::Update(
    const float deltaTime,
    const Vector3 &ballWorldPosition,
    const Vector3 &ballVelocityWorld,
    const float ballRadiusWorld,
    const bool IsOrbitControlActive,
    const float MouseDeltaX,
    const float MouseDeltaY
)
{
    if (m_followCamera == nullptr)
    {
        return;
    }

    Vector3 BallVelocityHorizontal = ballVelocityWorld;
    BallVelocityHorizontal.y = 0.0f;
    const float BallVelocityHorizontalLengthSquared = BallVelocityHorizontal.LengthSquared();

    if (!m_hasInitializedYaw)
    {
        if (BallVelocityHorizontalLengthSquared > 1.0e-6f)
        {
            BallVelocityHorizontal.Normalize();
            m_cameraYawRadians = std::atan2(BallVelocityHorizontal.x, BallVelocityHorizontal.z);
            m_hasInitializedYaw = true;
        }
        else
        {
            m_cameraYawRadians = 0.0f;
        }
    }

    float FrameMouseDeltaX = 0.0f;
    float FrameMouseDeltaY = 0.0f;
    if (m_hasPreviousMouseSample)
    {
        FrameMouseDeltaX = MouseDeltaX - m_previousMouseDeltaX;
        FrameMouseDeltaY = MouseDeltaY - m_previousMouseDeltaY;
    }
    m_previousMouseDeltaX = MouseDeltaX;
    m_previousMouseDeltaY = MouseDeltaY;
    m_hasPreviousMouseSample = true;

    if (IsOrbitControlActive)
    {
        constexpr float MouseYawSensitivity = 0.0036f;
        constexpr float MousePitchSensitivity = 0.0024f;
        m_cameraYawRadians = WrapAngleRadians(m_cameraYawRadians + FrameMouseDeltaX * MouseYawSensitivity);
        m_cameraPitchRadians = (std::clamp)(
            m_cameraPitchRadians - FrameMouseDeltaY * MousePitchSensitivity,
            -0.92f,
            0.48f
        );
    }

    Vector3 CameraForwardDirectionXZ{};
    CameraForwardDirectionXZ.x = std::sin(m_cameraYawRadians);
    CameraForwardDirectionXZ.y = 0.0f;
    CameraForwardDirectionXZ.z = std::cos(m_cameraYawRadians);
    m_followCamera->SetMovementDirectionXZ(CameraForwardDirectionXZ);

    constexpr float CameraOrbitRadius = 26.0f;
    constexpr float CameraHeightBias = 9.0f;
    const float CameraOffsetVertical = CameraHeightBias + std::sin(m_cameraPitchRadians) * CameraOrbitRadius;
    const float CameraOffsetBackward = -std::cos(m_cameraPitchRadians) * CameraOrbitRadius;
    m_followCamera->SetOffsetFromTarget(Vector3(0.0f, CameraOffsetVertical, CameraOffsetBackward));

    m_followCamera->SetTarget(ballWorldPosition);
    m_followCamera->SetBallRadiusForZoom(ballRadiusWorld);
    m_followCamera->Update(deltaTime);
}

float KatamariCameraController::WrapAngleRadians(const float angleRadians) noexcept
{
    constexpr float Pi = 3.14159265358979323846f;
    constexpr float TwoPi = Pi * 2.0f;

    float Wrapped = std::fmod(angleRadians, TwoPi);
    if (Wrapped > Pi)
    {
        Wrapped -= TwoPi;
    }
    if (Wrapped < -Pi)
    {
        Wrapped += TwoPi;
    }
    return Wrapped;
}

