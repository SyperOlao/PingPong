#include "Game/Katamari/KatamariLevelSetup.h"

#include "Core/App/AppContext.h"
#include "Core/Gameplay/Entity.h"
#include "Core/Gameplay/GameplayComponents.h"
#include "Core/Gameplay/Scene.h"
#include "Core/Graphics/ModelAsset.h"
#include "Core/Physics/Collision3D/AxisAlignedBox3D.h"
#include "Game/Katamari/Data/KatamariPickupCatalog.h"
#include "Game/Katamari/Data/PickupArchetype.h"
#include "Game/Katamari/KatamariTypes.h"
#include "Game/Katamari/Systems/KatamariSpawner.h"

#include <array>
#include <memory>

using DirectX::SimpleMath::Vector3;

float KatamariLevelSetup::SphereVolumeFromRadius(const float radius) noexcept
{
    constexpr float FourThirdsPi = 4.1887902047863905f;
    return FourThirdsPi * radius * radius * radius;
}

void KatamariLevelSetup::CreateStaticWorldCollision(Scene &scene, KatamariGameConfig const &config)
{
    const float halfExtent = config.PlayfieldHalfExtent;
    const float wallThickness = config.WallThickness;
    const float wallHeight = config.WallHeight;
    const float floorHalfThickness = config.FloorColliderHalfThickness;
    const float wallHalfHeight = wallHeight * 0.5f;
    const float span = halfExtent + wallThickness;

    Entity floor = scene.CreateEntity();
    TransformComponent floorTransform{};
    floorTransform.Local.Position = Vector3(0.0f, -floorHalfThickness, 0.0f);
    floor.AddTransformComponent(floorTransform);
    BoxColliderComponent floorBox{};
    floorBox.LocalCenter = Vector3::Zero;
    floorBox.HalfExtents = Vector3(halfExtent + wallThickness, floorHalfThickness, halfExtent + wallThickness);
    floorBox.CollisionLayer = KatamariCollisionLayer::WorldStatic;
    floorBox.CollidesWithMask = 1u << KatamariCollisionLayer::PlayerBall;
    floorBox.IsStatic = true;
    floor.AddBoxColliderComponent(floorBox);

    auto makeWall = [&](Vector3 const &center, Vector3 const &halfExtents) {
        Entity wall = scene.CreateEntity();
        TransformComponent wallTransform{};
        wallTransform.Local.Position = center;
        wall.AddTransformComponent(wallTransform);
        BoxColliderComponent wallBox{};
        wallBox.LocalCenter = Vector3::Zero;
        wallBox.HalfExtents = halfExtents;
        wallBox.CollisionLayer = KatamariCollisionLayer::WorldStatic;
        wallBox.CollidesWithMask = 1u << KatamariCollisionLayer::PlayerBall;
        wallBox.IsStatic = true;
        wall.AddBoxColliderComponent(wallBox);
    };

    const float wallHalfThickness = wallThickness * 0.5f;
    makeWall(
        Vector3(halfExtent + wallHalfThickness, wallHalfHeight, 0.0f),
        Vector3(wallHalfThickness, wallHalfHeight, span)
    );
    makeWall(
        Vector3(-halfExtent - wallHalfThickness, wallHalfHeight, 0.0f),
        Vector3(wallHalfThickness, wallHalfHeight, span)
    );
    makeWall(
        Vector3(0.0f, wallHalfHeight, halfExtent + wallHalfThickness),
        Vector3(span, wallHalfHeight, wallHalfThickness)
    );
    makeWall(
        Vector3(0.0f, wallHalfHeight, -halfExtent - wallHalfThickness),
        Vector3(span, wallHalfHeight, wallHalfThickness)
    );

    const float collisionMargin = 40.0f;
    AxisAlignedBox3D worldBounds{};
    worldBounds.Min = Vector3(
        -halfExtent - wallThickness - collisionMargin,
        -floorHalfThickness - wallHeight - collisionMargin,
        -halfExtent - wallThickness - collisionMargin
    );
    worldBounds.Max = Vector3(
        halfExtent + wallThickness + collisionMargin,
        wallHeight + collisionMargin,
        halfExtent + wallThickness + collisionMargin
    );
    scene.SetCollisionWorldBounds(worldBounds);
    scene.SetCollisionCellSize(config.CollisionCellSize);
}

void KatamariLevelSetup::CreateStaticObstacles(
    Scene &scene,
    KatamariWorldContext &world,
    KatamariGameConfig const &config,
    std::shared_ptr<ModelAsset> const &cubeModel,
    std::shared_ptr<ModelAsset> const &triangularPrismModel
)
{
    struct ObstacleDefinition final
    {
        KatamariObstacleShape Shape{KatamariObstacleShape::Cube};
        Vector3 Position{};
        Vector3 Scale{1.0f, 1.0f, 1.0f};
        float YawRadians{0.0f};
        DirectX::SimpleMath::Color BaseColor{1.0f, 1.0f, 1.0f, 1.0f};
        DirectX::SimpleMath::Color EmissiveColor{0.0f, 0.0f, 0.0f, 1.0f};
    };

    const float halfExtent = config.PlayfieldHalfExtent;
    const std::array<ObstacleDefinition, 6> obstacles =
    {{
        {
            KatamariObstacleShape::Cube,
            Vector3(-halfExtent * 0.55f, 0.0f, -halfExtent * 0.28f),
            Vector3(8.0f, 8.0f, 8.0f),
            0.32f,
            DirectX::SimpleMath::Color(0.1f, 0.95f, 0.95f, 1.0f),
            DirectX::SimpleMath::Color(0.18f, 0.9f, 0.85f, 1.0f)
        },
        {
            KatamariObstacleShape::TriangularPrism,
            Vector3(-halfExtent * 0.24f, 0.0f, halfExtent * 0.48f),
            Vector3(10.0f, 12.0f, 6.0f),
            -0.55f,
            DirectX::SimpleMath::Color(1.0f, 0.28f, 0.72f, 1.0f),
            DirectX::SimpleMath::Color(0.95f, 0.12f, 0.62f, 1.0f)
        },
        {
            KatamariObstacleShape::Cube,
            Vector3(halfExtent * 0.18f, 0.0f, -halfExtent * 0.5f),
            Vector3(7.0f, 11.0f, 7.0f),
            -0.7f,
            DirectX::SimpleMath::Color(0.55f, 1.0f, 0.2f, 1.0f),
            DirectX::SimpleMath::Color(0.32f, 0.95f, 0.16f, 1.0f)
        },
        {
            KatamariObstacleShape::TriangularPrism,
            Vector3(halfExtent * 0.56f, 0.0f, halfExtent * 0.14f),
            Vector3(12.0f, 9.0f, 8.0f),
            0.92f,
            DirectX::SimpleMath::Color(1.0f, 0.78f, 0.18f, 1.0f),
            DirectX::SimpleMath::Color(1.0f, 0.56f, 0.08f, 1.0f)
        },
        {
            KatamariObstacleShape::Cube,
            Vector3(-halfExtent * 0.08f, 0.0f, -halfExtent * 0.08f),
            Vector3(6.0f, 14.0f, 6.0f),
            0.0f,
            DirectX::SimpleMath::Color(0.48f, 0.58f, 1.0f, 1.0f),
            DirectX::SimpleMath::Color(0.28f, 0.36f, 1.0f, 1.0f)
        },
        {
            KatamariObstacleShape::TriangularPrism,
            Vector3(halfExtent * 0.36f, 0.0f, halfExtent * 0.55f),
            Vector3(8.0f, 10.0f, 10.0f),
            -1.12f,
            DirectX::SimpleMath::Color(0.96f, 0.2f, 1.0f, 1.0f),
            DirectX::SimpleMath::Color(0.68f, 0.14f, 1.0f, 1.0f)
        }
    }};

    world.StaticObstacles.clear();
    world.StaticObstacles.reserve(obstacles.size());

    for (const ObstacleDefinition &definition : obstacles)
    {
        Entity obstacle = scene.CreateEntity();

        TransformComponent transform{};
        transform.Local.Position = Vector3(
            definition.Position.x,
            definition.Scale.y * 0.5f,
            definition.Position.z
        );
        transform.Local.RotationEulerRad = Vector3(0.0f, definition.YawRadians, 0.0f);
        transform.Local.Scale = definition.Scale;
        obstacle.AddTransformComponent(transform);

        std::shared_ptr<ModelAsset> const &visualModel =
            definition.Shape == KatamariObstacleShape::TriangularPrism
                ? triangularPrismModel
                : cubeModel;
        if (visualModel != nullptr && visualModel->IsLoaded())
        {
            ModelComponent modelComponent{};
            modelComponent.Asset = visualModel;
            modelComponent.Visible = true;
            modelComponent.CastsShadow = true;
            obstacle.AddModelComponent(modelComponent);
        }

        BoxColliderComponent box{};
        box.LocalCenter = Vector3::Zero;
        box.HalfExtents = Vector3(0.5f, 0.5f, 0.5f);
        box.CollisionLayer = KatamariCollisionLayer::WorldStatic;
        box.CollidesWithMask = 1u << KatamariCollisionLayer::PlayerBall;
        box.IsTrigger = false;
        box.IsStatic = true;
        obstacle.AddBoxColliderComponent(box);

        TagComponent tag{};
        tag.TagId = KatamariTagId::StaticObstacle;
        tag.TagText = "StaticObstacle";
        obstacle.AddTagComponent(tag);

        KatamariStaticObstacleRecord record{};
        record.Entity = obstacle.GetId();
        record.Shape = definition.Shape;
        record.Material.BaseColor = definition.BaseColor;
        record.Material.ReceiveLighting = true;
        record.Material.AmbientFactor = 0.08f;
        record.Material.EmissiveColor = definition.EmissiveColor;
        record.Material.SpecularColor = DirectX::SimpleMath::Color(0.45f, 0.45f, 0.5f, 1.0f);
        record.Material.SpecularPower = 38.0f;

        MaterialComponent materialComponent{};
        materialComponent.Parameters = record.Material;
        obstacle.AddMaterialComponent(materialComponent);

        world.StaticObstacles.push_back(record);
    }
}

void KatamariLevelSetup::CreatePlayerBall(
    Scene &scene,
    KatamariWorldContext &world,
    std::shared_ptr<ModelAsset> const &ballMeshModel
)
{
    if (world.Config == nullptr)
    {
        return;
    }

    KatamariGameConfig const &config = *world.Config;
    Entity ball = scene.CreateEntity();
    world.BallEntityId = ball.GetId();

    TransformComponent transform{};
    transform.Local.Position = Vector3(0.0f, config.InitialBallRadius, 0.0f);

    world.BallMeshReferenceRadius = 1.0f;
    if (ballMeshModel != nullptr && ballMeshModel->IsLoaded())
    {
        world.BallMeshReferenceRadius = (std::max)(ballMeshModel->GetMergedBoundingSphere().Radius, 1.0e-4f);
        const float initialUniformScale = config.InitialBallRadius / world.BallMeshReferenceRadius;
        transform.Local.Scale = Vector3(initialUniformScale, initialUniformScale, initialUniformScale);
        ModelComponent modelComponent{};
        modelComponent.Asset = ballMeshModel;
        modelComponent.Visible = true;
        modelComponent.CastsShadow = true;
        ball.AddModelComponent(modelComponent);

        MaterialComponent materialComponent{};
        materialComponent.Parameters.BaseColor = DirectX::SimpleMath::Color(0.95f, 0.35f, 0.2f, 1.0f);
        materialComponent.Parameters.ReceiveLighting = true;
        materialComponent.Parameters.AmbientFactor = 0.1f;
        materialComponent.Parameters.SpecularColor = DirectX::SimpleMath::Color(1.0f, 0.96f, 0.88f, 1.0f);
        materialComponent.Parameters.SpecularPower = 22.0f;
        ball.AddMaterialComponent(materialComponent);
    }

    ball.AddTransformComponent(transform);

    VelocityComponent velocity{};
    ball.AddVelocityComponent(velocity);

    SphereColliderComponent sphere{};
    if (ballMeshModel != nullptr && ballMeshModel->IsLoaded())
    {
        sphere.Radius = world.BallMeshReferenceRadius;
    }
    else
    {
        sphere.Radius = config.InitialBallRadius;
    }
    sphere.CollisionLayer = KatamariCollisionLayer::PlayerBall;
    sphere.CollidesWithMask = (1u << KatamariCollisionLayer::WorldStatic) | (1u << KatamariCollisionLayer::Pickup);
    sphere.IsTrigger = false;
    ball.AddSphereColliderComponent(sphere);

    TagComponent tag{};
    tag.TagId = KatamariTagId::PlayerBall;
    tag.TagText = "PlayerBall";
    ball.AddTagComponent(tag);

    world.BallRadius = config.InitialBallRadius;
    world.BallVolume = SphereVolumeFromRadius(config.InitialBallRadius);
    world.BallVisualRadius = world.BallRadius;
}

void KatamariLevelSetup::SpawnPlayerBallAndPickups(
    Scene &scene,
    AppContext &context,
    KatamariWorldContext &world,
    KatamariGameConfig const &config,
    std::vector<PickupArchetype> const &archetypes,
    std::shared_ptr<ModelAsset> const &ballMeshModel
)
{
    world.Config = &config;
    world.Random.seed(config.RandomSeed);
    CreatePlayerBall(scene, world, ballMeshModel);

    KatamariSpawner spawner{};
    spawner.SpawnPickups(scene, context, world, archetypes);
}
