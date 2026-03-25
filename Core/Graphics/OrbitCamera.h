#ifndef PINGPONG_ORBITCAMERA_H
#define PINGPONG_ORBITCAMERA_H

#include "Core/Graphics/Camera.h"

class OrbitCamera final : public Camera
{
public:
    void SetTarget(const DirectX::SimpleMath::Vector3 &target) noexcept;

    void SetYawPitchRadius(float yaw, float pitch, float radius) noexcept;

    [[nodiscard]] DirectX::SimpleMath::Vector3 GetPosition() const noexcept override;

    [[nodiscard]] DirectX::SimpleMath::Matrix GetViewMatrix() const override;

private:
    [[nodiscard]] DirectX::SimpleMath::Vector3 CalculateEyePosition() const noexcept;

    DirectX::SimpleMath::Vector3 m_target{0.0f, 0.0f, 0.0f};
    float m_yaw{0.0f};
    float m_pitch{0.5f};
    float m_radius{30.0f};
};

#endif
