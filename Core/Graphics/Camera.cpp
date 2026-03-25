#include "Core/Graphics/Camera.h"

#include <DirectXMath.h>

#include <algorithm>
#include <cmath>
#include <stdexcept>

using DirectX::SimpleMath::Matrix;

namespace
{
    constexpr float kMinimumVerticalFieldOfViewDegrees = 1.0f;
    constexpr float kMaximumVerticalFieldOfViewDegrees = 169.0f;
    constexpr float kMinimumNearPlane = 0.001f;
}

DirectX::SimpleMath::Matrix Camera::GetProjectionMatrix(const float aspectRatio) const
{
    if (!(aspectRatio > 0.0f) || std::isnan(aspectRatio) || std::isinf(aspectRatio))
    {
        throw std::invalid_argument("Camera::GetProjectionMatrix requires a finite positive aspect ratio.");
    }

    switch (m_projectionMode)
    {
        case ProjectionMode::PerspectiveOffCenter:
            return Matrix::CreatePerspectiveOffCenter(
                -aspectRatio,
                aspectRatio,
                -1.0f,
                1.0f,
                m_nearPlane,
                m_farPlane
            );

        case ProjectionMode::PerspectiveFov:
        default:
            return Matrix::CreatePerspectiveFieldOfView(
                DirectX::XMConvertToRadians(m_verticalFieldOfViewDegrees),
                aspectRatio,
                m_nearPlane,
                m_farPlane
            );
    }
}

void Camera::SetProjectionMode(const ProjectionMode mode) noexcept
{
    m_projectionMode = mode;
}

ProjectionMode Camera::GetProjectionMode() const noexcept
{
    return m_projectionMode;
}

void Camera::SetVerticalFieldOfViewDegrees(const float verticalFieldOfViewDegrees)
{
    m_verticalFieldOfViewDegrees = std::clamp(
        verticalFieldOfViewDegrees,
        kMinimumVerticalFieldOfViewDegrees,
        kMaximumVerticalFieldOfViewDegrees
    );
}

float Camera::GetVerticalFieldOfViewDegrees() const noexcept
{
    return m_verticalFieldOfViewDegrees;
}

void Camera::SetNearPlane(const float nearPlane)
{
    if (!(nearPlane > kMinimumNearPlane) || std::isnan(nearPlane) || std::isinf(nearPlane))
    {
        throw std::invalid_argument("Camera::SetNearPlane requires a finite value greater than the engine minimum.");
    }

    if (nearPlane >= m_farPlane)
    {
        throw std::invalid_argument("Camera::SetNearPlane requires near plane to be less than far plane.");
    }

    m_nearPlane = nearPlane;
}

void Camera::SetFarPlane(const float farPlane)
{
    if (!(farPlane > m_nearPlane) || std::isnan(farPlane) || std::isinf(farPlane))
    {
        throw std::invalid_argument("Camera::SetFarPlane requires a finite value greater than near plane.");
    }

    m_farPlane = farPlane;
}

void Camera::SetNearPlaneAndFarPlane(const float nearPlane, const float farPlane)
{
    if (!(nearPlane > kMinimumNearPlane) || std::isnan(nearPlane) || std::isinf(nearPlane))
    {
        throw std::invalid_argument("Camera::SetNearPlaneAndFarPlane requires a valid near plane.");
    }

    if (!(farPlane > nearPlane) || std::isnan(farPlane) || std::isinf(farPlane))
    {
        throw std::invalid_argument("Camera::SetNearPlaneAndFarPlane requires far plane greater than near plane.");
    }

    m_nearPlane = nearPlane;
    m_farPlane = farPlane;
}

float Camera::GetNearPlane() const noexcept
{
    return m_nearPlane;
}

float Camera::GetFarPlane() const noexcept
{
    return m_farPlane;
}
