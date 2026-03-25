#include "Core/Graphics/FollowCamera.h"

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

void FollowCamera::SetTarget(const Vector3 &target) noexcept
{
    m_target = target;
}

void FollowCamera::SetOffsetFromTarget(const Vector3 &offsetFromTarget) noexcept
{
    m_offsetFromTarget = offsetFromTarget;
}

Vector3 FollowCamera::GetPosition() const noexcept
{
    return m_target + m_offsetFromTarget;
}

Matrix FollowCamera::GetViewMatrix() const
{
    const Vector3 eyePosition = GetPosition();
    return Matrix::CreateLookAt(eyePosition, m_target, Vector3::Up);
}
