//
// Created by SyperOlao on 19.03.2026.
//
#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "SolarSystemScene.h"


#include <algorithm>
#include <DirectXMath.h>
using namespace DirectX::SimpleMath;

void SolarSystemScene::Initialize()
{
    CreateDemoSystem();
    ApplyTuning();
}

void SolarSystemScene::Update(const float deltaTime)
{
    ApplyTuning();

    for (const auto& root : m_roots)
    {
        root->Update(deltaTime);
        root->UpdateWorldMatrix(Matrix::Identity);
    }
}

void SolarSystemScene::SetTuning(const SolarSystemTuning& tuning) noexcept
{
    m_tuning = tuning;
}

const std::vector<std::shared_ptr<OrbitalBody>>& SolarSystemScene::GetRoots() const noexcept
{
    return m_roots;
}

void SolarSystemScene::CreateDemoSystem()
{
    m_roots.clear();

    const auto sun = std::make_shared<OrbitalBody>();
    sun->MeshType = BodyMeshType::Box;
    sun->VisualClass = BodyVisualClass::Star;
    sun->SurfaceType = BodySurfaceType::Star;
    sun->Scale = Vector3(5.2f, 5.2f, 5.2f);
    sun->BaseColor = Color(1.0f, 0.86f, 0.22f, 1.0f);
    sun->OrbitColor = Color(0.0f, 0.0f, 0.0f, 0.0f);
    sun->SelfRotationSpeed = 0.18f;
    sun->BaseSelfRotationSpeed = sun->SelfRotationSpeed;
    sun->ShowOrbit = false;

    auto makePlanet = [](
        const Vector3& scale,
        const Color& color,
        const Color& orbitColor,
        const BodySurfaceType surfaceType,
        const OrbitalParams& orbit,
        const float selfSpeed)
    {
        auto body = std::make_shared<OrbitalBody>();
        body->MeshType = BodyMeshType::Sphere;
        body->VisualClass = BodyVisualClass::Planet;
        body->SurfaceType = surfaceType;
        body->Scale = scale;
        body->BaseColor = color;
        body->OrbitColor = orbitColor;
        body->HasOrbit = true;
        body->Orbit = orbit;
        body->BaseOrbit = orbit;
        body->SelfRotationSpeed = selfSpeed;
        body->BaseSelfRotationSpeed = selfSpeed;
        body->ShowOrbit = true;
        return body;
    };

    auto makeMoon = [](
        const Vector3& scale,
        const Color& color,
        const Color& orbitColor,
        const BodySurfaceType surfaceType,
        const OrbitalParams& orbit,
        const float selfSpeed)
    {
        auto body = std::make_shared<OrbitalBody>();
        body->MeshType = BodyMeshType::Sphere;
        body->VisualClass = BodyVisualClass::Moon;
        body->SurfaceType = surfaceType;
        body->Scale = scale;
        body->BaseColor = color;
        body->OrbitColor = orbitColor;
        body->HasOrbit = true;
        body->Orbit = orbit;
        body->BaseOrbit = orbit;
        body->SelfRotationSpeed = selfSpeed;
        body->BaseSelfRotationSpeed = selfSpeed;
        body->ShowOrbit = true;
        return body;
    };

    const auto planet1 = makePlanet(
        Vector3(1.25f, 1.25f, 1.25f),
        Color(0.74f, 0.48f, 0.24f, 1.0f),              // rocky
        Color(0.38f, 0.28f, 0.18f, 1.0f),
        BodySurfaceType::Rocky,
        {13.0f, 0.06f, 7.0f, 0.70f, 0.2f},
        1.2f
    );

    const auto moon1 = makeMoon(
        Vector3(0.32f, 0.32f, 0.32f),
        Color(0.78f, 0.78f, 0.80f, 1.0f),
        Color(0.42f, 0.42f, 0.46f, 1.0f),
        BodySurfaceType::Rocky,
        {2.1f, 0.02f, 16.0f, 2.4f, 0.5f},
        1.8f
    );

    const auto planet2 = makePlanet(
        Vector3(2.15f, 2.15f, 2.15f),
        Color(0.26f, 0.76f, 0.52f, 1.0f),
        Color(0.18f, 0.40f, 0.30f, 1.0f),
        BodySurfaceType::GasGiant,
        {21.0f, 0.10f, 11.0f, 0.42f, 1.1f},
        0.55f
    );

    const auto moon2 = makeMoon(
        Vector3(0.45f, 0.45f, 0.45f),
        Color(0.62f, 0.72f, 0.88f, 1.0f),
        Color(0.36f, 0.46f, 0.62f, 1.0f),
        BodySurfaceType::Ice,
        {3.0f, 0.03f, 20.0f, 1.65f, 0.3f},
        1.5f
    );

    const auto planet3 = makePlanet(
        Vector3(1.75f, 1.75f, 1.75f),
        Color(0.58f, 0.66f, 0.92f, 1.0f),
        Color(0.26f, 0.36f, 0.62f, 1.0f),
        BodySurfaceType::Ice,
        {30.0f, 0.14f, 4.0f, 0.28f, 2.0f},
        0.35f
    );

    const auto moon3 = makeMoon(
        Vector3(0.38f, 0.38f, 0.38f),
        Color(0.70f, 0.70f, 0.73f, 1.0f),
        Color(0.40f, 0.40f, 0.46f, 1.0f),
        BodySurfaceType::Rocky,
        {3.6f, 0.05f, 12.0f, 1.5f, 0.7f},
        1.2f
    );

    planet1->AddChild(moon1);
    planet2->AddChild(moon2);
    planet3->AddChild(moon3);

    sun->AddChild(planet1);
    sun->AddChild(planet2);
    sun->AddChild(planet3);

    m_roots.push_back(sun);
}

void SolarSystemScene::ApplyTuning()
{
    for (const auto& root : m_roots)
    {
        ApplyTuningRecursive(*root);
    }
}

void SolarSystemScene::ApplyTuningRecursive(OrbitalBody& body)
{
    switch (body.VisualClass)
    {
        case BodyVisualClass::Star:
            body.SelfRotationSpeed = body.BaseSelfRotationSpeed;
            break;

        case BodyVisualClass::Planet:
            body.SelfRotationSpeed = body.BaseSelfRotationSpeed * m_tuning.PlanetRotationScale;
            if (body.HasOrbit)
            {
                body.Orbit = body.BaseOrbit;
                body.Orbit.AngularSpeed = body.BaseOrbit.AngularSpeed * m_tuning.PlanetOrbitSpeedScale;
                body.Orbit.SemiMajorAxis = body.BaseOrbit.SemiMajorAxis * m_tuning.PlanetOrbitRadiusScale;
                body.Orbit.Eccentricity = std::clamp(
                    body.BaseOrbit.Eccentricity * m_tuning.OrbitEccentricityScale,
                    0.0f,
                    0.92f
                );
            }
            break;

        case BodyVisualClass::Moon:
            body.SelfRotationSpeed = body.BaseSelfRotationSpeed * m_tuning.MoonRotationScale;
            if (body.HasOrbit)
            {
                body.Orbit = body.BaseOrbit;
                body.Orbit.AngularSpeed = body.BaseOrbit.AngularSpeed * m_tuning.MoonOrbitSpeedScale;
                body.Orbit.SemiMajorAxis = body.BaseOrbit.SemiMajorAxis * m_tuning.MoonOrbitRadiusScale;
                body.Orbit.Eccentricity = std::clamp(
                    body.BaseOrbit.Eccentricity * m_tuning.OrbitEccentricityScale,
                    0.0f,
                    0.92f
                );
            }
            break;
    }

    for (const auto& child : body.GetChildren())
    {
        ApplyTuningRecursive(*child);
    }
}