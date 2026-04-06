#include "Game/Katamari/KatamariLevelSetup.h"

#include "Core/App/AppContext.h"
#include "Core/Gameplay/Entity.h"
#include "Core/Gameplay/GameplayComponents.h"
#include "Core/Gameplay/Scene.h"
#include "Core/Physics/Collision3D/AxisAlignedBox3D.h"
#include "Game/Katamari/Data/KatamariPickupCatalog.h"
#include "Game/Katamari/Data/PickupArchetype.h"
#include "Game/Katamari/KatamariTypes.h"
#include "Game/Katamari/Systems/KatamariSpawner.h"

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

void KatamariLevelSetup::CreatePlayerBall(Scene &scene, KatamariWorldContext &world)
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
    ball.AddTransformComponent(transform);

    VelocityComponent velocity{};
    ball.AddVelocityComponent(velocity);

    SphereColliderComponent sphere{};
    sphere.Radius = config.InitialBallRadius;
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
    std::vector<PickupArchetype> const &archetypes
)
{
    world.Config = &config;
    world.Random.seed(config.RandomSeed);
    CreatePlayerBall(scene, world);

    KatamariSpawner spawner{};
    spawner.SpawnPickups(scene, context, world, archetypes);
}
