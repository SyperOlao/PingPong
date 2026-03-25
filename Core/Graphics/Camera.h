#ifndef PINGPONG_CAMERA_H
#define PINGPONG_CAMERA_H

#include <SimpleMath.h>

#include "ProjectionMode.h"

class Camera
{
public:
    virtual ~Camera() = default;

    [[nodiscard]] virtual DirectX::SimpleMath::Matrix GetViewMatrix() const = 0;

    [[nodiscard]] virtual DirectX::SimpleMath::Vector3 GetPosition() const = 0;

    [[nodiscard]] DirectX::SimpleMath::Matrix GetProjectionMatrix(float aspectRatio) const;

    void SetProjectionMode(ProjectionMode mode) noexcept;

    [[nodiscard]] ProjectionMode GetProjectionMode() const noexcept;

    void SetVerticalFieldOfViewDegrees(float verticalFieldOfViewDegrees);

    [[nodiscard]] float GetVerticalFieldOfViewDegrees() const noexcept;

    void SetNearPlane(float nearPlane);

    void SetFarPlane(float farPlane);

    void SetNearPlaneAndFarPlane(float nearPlane, float farPlane);

    [[nodiscard]] float GetNearPlane() const noexcept;

    [[nodiscard]] float GetFarPlane() const noexcept;

protected:
    ProjectionMode m_projectionMode{ProjectionMode::PerspectiveFov};
    float m_verticalFieldOfViewDegrees{60.0f};
    float m_nearPlane{0.1f};
    float m_farPlane{2000.0f};
};

#endif
