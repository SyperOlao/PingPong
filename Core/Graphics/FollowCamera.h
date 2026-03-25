#ifndef PINGPONG_FOLLOWCAMERA_H
#define PINGPONG_FOLLOWCAMERA_H

#include "Core/Graphics/Camera.h"

class FollowCamera final : public Camera
{
public:
    void SetTarget(const DirectX::SimpleMath::Vector3 &target) noexcept;

    void SetOffsetFromTarget(const DirectX::SimpleMath::Vector3 &offsetFromTarget) noexcept;

    [[nodiscard]] DirectX::SimpleMath::Vector3 GetPosition() const noexcept override;

    [[nodiscard]] DirectX::SimpleMath::Matrix GetViewMatrix() const override;

private:
    DirectX::SimpleMath::Vector3 m_target{0.0f, 0.0f, 0.0f};
    DirectX::SimpleMath::Vector3 m_offsetFromTarget{0.0f, 8.0f, 20.0f};
};

#endif
