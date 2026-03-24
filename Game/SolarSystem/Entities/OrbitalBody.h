//
// Created by SyperOlao on 19.03.2026.
//

#ifndef PINGPONG_ORBITALBODY_H
#define PINGPONG_ORBITALBODY_H

#include <SimpleMath.h>
#include <memory>
#include <vector>
#include "../Systems/OrbitMath.h"

enum class BodyMeshType : std::uint8_t
{
    Sphere = 0,
    Box
};

enum class BodyVisualClass : std::uint8_t
{
    Star = 0,
    Planet,
    Moon
};

class OrbitalBody final
{
public:
    OrbitalBody() = default;

    void Update(float deltaTime);
    void UpdateWorldMatrix(const DirectX::SimpleMath::Matrix& parentWorld);

    void AddChild(std::shared_ptr<OrbitalBody> child);

    [[nodiscard]] const DirectX::SimpleMath::Matrix& GetWorldMatrix() const noexcept;
    [[nodiscard]] const std::vector<std::shared_ptr<OrbitalBody>>& GetChildren() const noexcept;


    BodyMeshType MeshType{BodyMeshType::Sphere};
    BodyVisualClass VisualClass{BodyVisualClass::Planet};

    DirectX::SimpleMath::Vector3 Scale{1.0f, 1.0f, 1.0f};

    DirectX::SimpleMath::Color BaseColor{1.0f, 1.0f, 1.0f, 1.0f};
    float EmissiveStrength{0.0f};
    bool ShowOrbit{true};

    OrbitalParams Orbit{};
    bool HasOrbit{false};

    float SelfRotationAngle{0.0f};
    float SelfRotationSpeed{0.0f};

private:
    float m_currentMeanAnomaly{0.0f};
    DirectX::SimpleMath::Matrix m_world{DirectX::SimpleMath::Matrix::Identity};

    std::vector<std::shared_ptr<OrbitalBody>> m_children;
};


#endif //PINGPONG_ORBITALBODY_H