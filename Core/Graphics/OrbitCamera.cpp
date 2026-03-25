#include "Core/Graphics/OrbitCamera.h"

#include "Core/Math/SpatialMath.h"

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

void OrbitCamera::SetTarget(const Vector3 &target) noexcept
{
    m_target = target;
}

void OrbitCamera::SetYawPitchRadius(const float yaw, const float pitch, const float radius) noexcept
{
    m_yaw = yaw;
    m_pitch = pitch;
    m_radius = radius;
}

Vector3 OrbitCamera::GetPosition() const noexcept
{
    return CalculateEyePosition();
}

Matrix OrbitCamera::GetViewMatrix() const
{
    const Vector3 eyePosition = CalculateEyePosition();
    return Matrix::CreateLookAt(eyePosition, m_target, Vector3::Up);
}

Vector3 OrbitCamera::CalculateEyePosition() const noexcept
{
    const Vector3 offset = SpatialMath::ForwardFromYawPitchRadians(m_yaw, m_pitch) * m_radius;
    return m_target + offset;
}
