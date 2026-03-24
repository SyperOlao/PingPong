//
// Created by SyperOlao on 19.03.2026.
//
#include "OrbitMath.h"
#include "../../../Core/Math/MathHelpers.h"
#include <cmath>
#include <DirectXMath.h>

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

namespace
{
    constexpr float Pi = 3.14159265358979323846f;
    constexpr float TwoPi = 2.0f * Pi;
}

float OrbitMath::SolveKepler(const float meanAnomaly, const float eccentricity) noexcept
{
    float M = std::fmod(meanAnomaly, TwoPi);
    if (M < 0.0f)
    {
        M += TwoPi;
    }

    float E = (eccentricity < 0.8f) ? M : Pi;

    for (int i = 0; i < 32; ++i)
    {
        const float f = E - eccentricity * std::sin(E) - M;
        const float fp = 1.0f - eccentricity * std::cos(E);
        const float delta = -f / fp;

        E += delta;

        if (std::fabs(delta) < 1e-6f)
        {
            break;
        }
    }

    return E;
}

Vector3 OrbitMath::CalculateLocalPosition(const OrbitalParams& orbit, const float meanAnomaly) noexcept
{
    const float E = SolveKepler(meanAnomaly, orbit.Eccentricity);

    const float xOrb = orbit.SemiMajorAxis * (std::cos(E) - orbit.Eccentricity);
    const float zOrb = orbit.SemiMajorAxis * std::sqrt(1.0f - orbit.Eccentricity * orbit.Eccentricity) * std::sin(E);

    const float inc = DirectX::XMConvertToRadians(orbit.InclinationDeg);

    const float x = xOrb;
    const float y = zOrb * std::sin(inc);
    const float z = zOrb * std::cos(inc);

    return Vector3(x, y, z);
}

Matrix OrbitMath::CalculateOrbitMatrix(const OrbitalParams& orbit, const float meanAnomaly) noexcept
{
    const Vector3 localPosition = CalculateLocalPosition(orbit, meanAnomaly);
    return Matrix::CreateTranslation(localPosition);
}