#ifndef PINGPONG_FPSCAMERA_H
#define PINGPONG_FPSCAMERA_H

#include "Core/Graphics/Camera.h"

class FpsCamera final : public Camera
{
public:
    void SetPosition(const DirectX::SimpleMath::Vector3 &position) noexcept;

    [[nodiscard]] DirectX::SimpleMath::Vector3 GetPosition() const noexcept override;

    void SetRotation(float yaw, float pitch) noexcept;

    void AddRotation(float yawDelta, float pitchDelta) noexcept;

    void MoveForward(float distance) noexcept;

    void MoveRight(float distance) noexcept;

    [[nodiscard]] DirectX::SimpleMath::Matrix GetViewMatrix() const override;

private:
    [[nodiscard]] DirectX::SimpleMath::Vector3 GetForward() const noexcept;

    [[nodiscard]] DirectX::SimpleMath::Vector3 GetRight() const noexcept;

    DirectX::SimpleMath::Vector3 m_position{0.0f, 3.0f, -20.0f};
    float m_yaw{0.0f};
    float m_pitch{0.0f};
};

#endif
