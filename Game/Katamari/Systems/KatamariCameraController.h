#ifndef PINGPONG_KATAMARICAMERACONTROLLER_H
#define PINGPONG_KATAMARICAMERACONTROLLER_H

#include <SimpleMath.h>

class FollowCamera;

class KatamariCameraController final
{
public:
    void Initialize(FollowCamera &followCamera) noexcept;

    void Update(
        float deltaTime,
        const DirectX::SimpleMath::Vector3 &ballWorldPosition,
        const DirectX::SimpleMath::Vector3 &ballVelocityWorld,
        float ballRadiusWorld,
        bool IsOrbitControlActive,
        float MouseDeltaX,
        float MouseDeltaY
    );

private:
    [[nodiscard]] static float WrapAngleRadians(float angleRadians) noexcept;

private:
    FollowCamera *m_followCamera{nullptr};
    float m_cameraYawRadians{0.0f};
    float m_cameraPitchRadians{0.0f};
    bool m_hasInitializedYaw{false};
    float m_previousMouseDeltaX{0.0f};
    float m_previousMouseDeltaY{0.0f};
    bool m_hasPreviousMouseSample{false};
};

#endif
