//
// Created by SyperOlao on 19.03.2026.
//

#ifndef PINGPONG_ORBITMATH_H
#define PINGPONG_ORBITMATH_H
#include <SimpleMath.h>

struct OrbitalParams final
{
    float SemiMajorAxis{1.0f};
    float Eccentricity{0.0f};
    float InclinationDeg{0.0f};
    float AngularSpeed{0.0f};
    float Phase{0.0f};
};

class OrbitMath final
{
public:
    [[nodiscard]] static float SolveKepler(float meanAnomaly, float eccentricity) noexcept;

    [[nodiscard]] static DirectX::SimpleMath::Vector3 CalculateLocalPosition(
        const OrbitalParams& orbit,
        float meanAnomaly
    ) noexcept;

    [[nodiscard]] static DirectX::SimpleMath::Matrix CalculateOrbitMatrix(
        const OrbitalParams& orbit,
        float meanAnomaly
    ) noexcept;
};
#endif //PINGPONG_ORBITMATH_H
