#include "Core/Graphics/FpsCamera.h"

#include "Core/Math/SpatialMath.h"

#include <algorithm>
#include <cmath>

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

namespace
{
    constexpr float kPitchLimit = 1.55334306f;
}

void FpsCamera::SetPosition(const Vector3 &position) noexcept
{
    m_position = position;
}

Vector3 FpsCamera::GetPosition() const noexcept
{
    return m_position;
}

void FpsCamera::SetRotation(const float yaw, const float pitch) noexcept
{
    m_yaw = yaw;
    m_pitch = std::clamp(pitch, -kPitchLimit, kPitchLimit);
}

void FpsCamera::AddRotation(const float yawDelta, const float pitchDelta) noexcept
{
    m_yaw += yawDelta;
    m_pitch = std::clamp(m_pitch + pitchDelta, -kPitchLimit, kPitchLimit);
}

void FpsCamera::MoveForward(const float distance) noexcept
{
    m_position += GetForward() * distance;
}

void FpsCamera::MoveRight(const float distance) noexcept
{
    m_position += GetRight() * distance;
}

Matrix FpsCamera::GetViewMatrix() const
{
    const Vector3 forward = GetForward();
    const Vector3 target = m_position + forward;
    return Matrix::CreateLookAt(m_position, target, Vector3::Up);
}

Vector3 FpsCamera::GetForward() const noexcept
{
    return SpatialMath::ForwardFromYawPitchRadians(m_yaw, m_pitch);
}

Vector3 FpsCamera::GetRight() const noexcept
{
    return SpatialMath::RightFromForwardAndWorldUp(GetForward(), Vector3::Up);
}
