//
// Created by SyperOlao on 17.03.2026.
//

#ifndef PINGPONG_MATHHELPERS_H
#define PINGPONG_MATHHELPERS_H

// General-purpose scalar and 2D helpers (clamp, lerp, random, 2D normalize/reflect).
// Vector3 TRS composition, basis axes, planes, and yaw/pitch direction: see SpatialMath.h.

#include <algorithm>
#include <concepts>
#include <random>
#include <SimpleMath.h>



class MathHelpers final
{
public:
    template <std::totally_ordered T>
    static constexpr T Clamp(const T value, const T minValue, const T maxValue) noexcept
    {
        return std::clamp(value, minValue, maxValue);
    }

    template <typename T>
    static constexpr T Lerp(const T& a, const T& b, const float alpha) noexcept
    {
        return a + (b - a) * alpha;
    }

    [[nodiscard]] static constexpr float DegToRad(const float degrees) noexcept
    {
        return degrees * 0.01745329251994329576923690768489f;
    }

    [[nodiscard]] static constexpr float RadToDeg(const float radians) noexcept
    {
        return radians * 57.295779513082320876798154814105f;
    }

    [[nodiscard]] static float RandomFloat(const float minValue, const float maxValue)
    {
        thread_local std::mt19937 generator{std::random_device{}()};
        std::uniform_real_distribution<float> distribution(minValue, maxValue);
        return distribution(generator);
    }

    [[nodiscard]] static float Sign(const float value) noexcept
    {
        if (value > 0.0f)
        {
            return 1.0f;
        }

        if (value < 0.0f)
        {
            return -1.0f;
        }

        return 0.0f;
    }

    [[nodiscard]] static DirectX::SimpleMath::Vector2 SafeNormalize(
        const DirectX::SimpleMath::Vector2& vector,
        const DirectX::SimpleMath::Vector2& fallback = DirectX::SimpleMath::Vector2{1.0f, 0.0f}
    ) noexcept
    {
        if (vector.LengthSquared() <= 0.000001f)
        {
            return fallback;
        }

        auto normalized = vector;
        normalized.Normalize();
        return normalized;
    }

    [[nodiscard]] static DirectX::SimpleMath::Vector2 Reflect(
        const DirectX::SimpleMath::Vector2& incoming,
        const DirectX::SimpleMath::Vector2& normal
    ) noexcept
    {
        const auto safeNormal = SafeNormalize(normal, DirectX::SimpleMath::Vector2{0.0f, 1.0f});
        return incoming - 2.0f * incoming.Dot(safeNormal) * safeNormal;
    }
};
#endif //PINGPONG_MATHHELPERS_H