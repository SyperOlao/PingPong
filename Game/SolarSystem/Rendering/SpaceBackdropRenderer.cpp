//
// Created by SyperOlao on 25.03.2026.
//
#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "SpaceBackdropRenderer.h"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <random>
#include <stdexcept>

#include "Core/Graphics/Camera.h"
#include "../../../Core/Graphics/Rendering/PrimitiveRenderer3D.h"

using DirectX::SimpleMath::Color;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

namespace {
    constexpr float kPi = std::numbers::pi_v<float>;

    float RandomRange(std::mt19937 &rng, const float minValue, const float maxValue) {
        std::uniform_real_distribution<float> distribution(minValue, maxValue);
        return distribution(rng);
    }

    int RandomIndex(std::mt19937 &rng, const int maxExclusive) {
        std::uniform_int_distribution<int> distribution(0, maxExclusive - 1);
        return distribution(rng);
    }
}

void SpaceBackdropRenderer::Initialize() {
    if (m_initialized) {
        return;
    }

    std::mt19937 rng(133742);

    m_stars.clear();
    m_stars.reserve(1400);

    GenerateDeepField(rng, 700);
    GenerateGalacticBand(rng, 550);
    GenerateAccentStars(rng, 120);

    m_initialized = true;
}

void SpaceBackdropRenderer::Render(
    PrimitiveRenderer3D &renderer,
    const Camera &camera,
    const Matrix &view,
    const Matrix &projection
) const {
    if (!m_initialized) {
        throw std::logic_error("SpaceBackdropRenderer::Render called before Initialize.");
    }

    (void) camera;

    const Matrix cameraWorld = view.Invert();
    const Vector3 cameraPosition = cameraWorld.Translation();

    for (const StarInstance &star: m_stars) {
        const Vector3 worldPosition = cameraPosition + star.Direction * star.Distance;
        const Matrix world =
                Matrix::CreateScale(star.Radius) *
                Matrix::CreateTranslation(worldPosition);

        renderer.DrawSphere(world, view, projection, star.Color);
    }
}

Vector3 SpaceBackdropRenderer::RandomUnitVector(std::mt19937 &rng) {
    const float z = RandomRange(rng, -1.0f, 1.0f);
    const float azimuth = RandomRange(rng, 0.0f, 2.0f * kPi);

    const float radial = std::sqrt(std::max(0.0f, 1.0f - z * z));
    const float x = radial * std::cos(azimuth);
    const float y = radial * std::sin(azimuth);

    return Vector3(x, y, z);
}

void SpaceBackdropRenderer::GenerateDeepField(std::mt19937 &rng, const int count) {
    static constexpr Color palette[] =
    {
        Color(0.92f, 0.95f, 1.0f, 1.0f),
        Color(0.82f, 0.90f, 1.0f, 1.0f),
        Color(1.0f, 0.95f, 0.86f, 1.0f),
        Color(0.88f, 0.88f, 1.0f, 1.0f)
    };

    for (int i = 0; i < count; ++i) {
        StarInstance star{};
        star.Direction = RandomUnitVector(rng);
        star.Distance = RandomRange(rng, 220.0f, 380.0f);
        star.Radius = RandomRange(rng, 0.08f, 0.22f);
        star.Color = palette[RandomIndex(rng, static_cast<int>(std::size(palette)))];


        if (RandomRange(rng, 0.0f, 1.0f) > 0.88f) {
            star.Radius *= RandomRange(rng, 1.8f, 3.4f);
        }

        m_stars.push_back(star);
    }
}

void SpaceBackdropRenderer::GenerateGalacticBand(std::mt19937 &rng, const int count) {
    static constexpr Color palette[] =
    {
        Color(0.58f, 0.68f, 1.0f, 1.0f),
        Color(0.72f, 0.82f, 1.0f, 1.0f),
        Color(0.84f, 0.88f, 1.0f, 1.0f),
        Color(0.72f, 0.62f, 1.0f, 1.0f),
        Color(1.0f, 0.78f, 0.92f, 1.0f)
    };

    for (int i = 0; i < count; ++i) {
        const float angle = RandomRange(rng, 0.0f, 2.0f * kPi);
        const float bandOffset = RandomRange(rng, -0.22f, 0.22f);
        const float thicknessNoise = RandomRange(rng, -0.06f, 0.06f);

        Vector3 direction{
            std::cos(angle),
            bandOffset + thicknessNoise,
            std::sin(angle)
        };
        direction.Normalize();

        StarInstance star{};
        star.Direction = direction;
        star.Distance = RandomRange(rng, 240.0f, 420.0f);
        star.Radius = RandomRange(rng, 0.07f, 0.18f);
        star.Color = palette[RandomIndex(rng, static_cast<int>(std::size(palette)))];

        if (RandomRange(rng, 0.0f, 1.0f) > 0.80f)
        {
            star.Radius *= RandomRange(rng, 1.6f, 2.8f);
        }

        m_stars.push_back(star);
    }
}

void SpaceBackdropRenderer::GenerateAccentStars(std::mt19937 &rng, const int count) {
    static constexpr Color palette[] =
    {
        Color(1.0f, 0.82f, 0.62f, 1.0f),
        Color(0.66f, 0.80f, 1.0f, 1.0f),
        Color(1.0f, 0.72f, 0.82f, 1.0f),
        Color(0.76f, 1.0f, 0.84f, 1.0f),
        Color(1.0f, 0.96f, 0.86f, 1.0f)
    };

    for (int i = 0; i < count; ++i) {
        StarInstance star{};
        star.Direction = RandomUnitVector(rng);
        star.Distance = RandomRange(rng, 180.0f, 320.0f);
        star.Radius = RandomRange(rng, 0.28f, 0.72f);
        star.Color = palette[RandomIndex(rng, static_cast<int>(std::size(palette)))];
        m_stars.push_back(star);
    }
}
