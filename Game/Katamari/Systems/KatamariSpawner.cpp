#include "Game/Katamari/Systems/KatamariSpawner.h"

#include "Core/App/AppContext.h"
#include "Core/Assets/AssetCache.h"
#include "Core/Gameplay/Entity.h"
#include "Core/Gameplay/GameplayComponents.h"
#include "Core/Gameplay/Scene.h"
#include "Core/Graphics/ModelAsset.h"
#include "Game/Katamari/KatamariTypes.h"

#include <algorithm>
#include <cmath>
#include <random>

using DirectX::SimpleMath::Vector3;

namespace
{
[[nodiscard]] float BaseLocalRadiusFromMergedBoundsOrOverride(
    std::shared_ptr<ModelAsset> const &modelAsset,
    float const collisionSphereRadiusOverride
)
{
    if (collisionSphereRadiusOverride > 0.0f)
    {
        return collisionSphereRadiusOverride;
    }

    if (modelAsset == nullptr || !modelAsset->IsLoaded())
    {
        return 0.35f;
    }

    const DirectX::BoundingSphere merged = modelAsset->GetMergedBoundingSphere();
    return (std::max)(merged.Radius, 0.2f);
}
}

void KatamariSpawner::SpawnPickups(
    Scene &scene,
    AppContext &context,
    KatamariWorldContext &world,
    std::vector<PickupArchetype> const &archetypes
) const
{
    if (world.Config == nullptr || archetypes.empty())
    {
        return;
    }

    AssetCache *const cache = context.Assets.Cache;
    if (cache == nullptr)
    {
        return;
    }

    world.Pickups.clear();
    world.TotalPickups = 0;
    world.CollectedCount = 0;

    const KatamariGameConfig &config = *world.Config;
    const unsigned int spawnCount = config.PickupSpawnCount;
    const unsigned int maxAttempts = config.SpawnMaxAttemptsPerPickup;
    const float separation = config.PickupSpawnMinSeparation;

    std::uniform_int_distribution<std::size_t> archetypeDistribution(0u, archetypes.size() - 1u);

    for (unsigned int spawnIndex = 0u; spawnIndex < spawnCount; ++spawnIndex)
    {
        PickupArchetype const &archetype = archetypes[archetypeDistribution(world.Random)];

        std::shared_ptr<ModelAsset> model = cache->LoadModel(archetype.ModelPath);
        if (model == nullptr || !model->IsLoaded())
        {
            continue;
        }

        const float uniformScale = (std::max)(archetype.VisualUniformScale, 1.0e-4f);
        const float baseLocalRadius = BaseLocalRadiusFromMergedBoundsOrOverride(model, archetype.CollisionSphereRadius);
        const float scaledLocalRadius = baseLocalRadius * archetype.CollisionRadiusScale;
        const float worldRadiusFromGeometry = scaledLocalRadius * uniformScale;
        const float pickupCollisionRadiusWorld = (std::max)(0.02f, worldRadiusFromGeometry + archetype.CollisionRadiusPadding);
        const float localColliderRadius = pickupCollisionRadiusWorld / uniformScale;

        Vector3 position{};
        bool placed = false;
        for (unsigned int attempt = 0u; attempt < maxAttempts; ++attempt)
        {
            position = SampleSpawnPosition(world);
            position.y = pickupCollisionRadiusWorld;
            if (!IsSpawnPositionTooClose(scene, world, position, separation + pickupCollisionRadiusWorld))
            {
                placed = true;
                break;
            }
        }

        if (!placed)
        {
            continue;
        }

        Entity pickup = scene.CreateEntity();

        TransformComponent transform{};
        transform.Local.Position = position;
        transform.Local.Scale = Vector3(
            archetype.VisualUniformScale,
            archetype.VisualUniformScale,
            archetype.VisualUniformScale
        );
        pickup.AddTransformComponent(transform);

        ModelComponent modelComponent{};
        modelComponent.Asset = std::move(model);
        modelComponent.Visible = true;
        modelComponent.CastsShadow = true;
        pickup.AddModelComponent(modelComponent);

        MaterialComponent materialComponent{};
        materialComponent.Parameters.AmbientFactor = 0.09f;
        materialComponent.Parameters.SpecularColor = DirectX::SimpleMath::Color(0.55f, 0.55f, 0.55f, 1.0f);
        materialComponent.Parameters.SpecularPower = 42.0f;
        pickup.AddMaterialComponent(materialComponent);

        SphereColliderComponent sphere{};
        sphere.LocalCenter = archetype.CollisionCenterOffsetLocal;
        sphere.Radius = localColliderRadius;
        sphere.CollisionLayer = KatamariCollisionLayer::Pickup;
        sphere.CollidesWithMask = 1u << KatamariCollisionLayer::PlayerBall;
        sphere.IsTrigger = false;
        pickup.AddSphereColliderComponent(sphere);

        TagComponent tag{};
        tag.TagId = KatamariTagId::Pickup;
        tag.TagText = archetype.DisplayName;
        pickup.AddTagComponent(tag);

        KatamariPickupRecord record{};
        record.AbsorbVolumeContribution = archetype.AbsorbVolumeContribution;
        record.MinimumBallRadiusToAbsorb = archetype.MinimumBallRadiusToAbsorb;
        record.Absorbed = false;
        world.Pickups[pickup.GetId()] = record;
        ++world.TotalPickups;
    }
}

Vector3 KatamariSpawner::SampleSpawnPosition(KatamariWorldContext &world) const
{
    KatamariGameConfig const *const config = world.Config;
    if (config == nullptr)
    {
        return Vector3::Zero;
    }

    std::uniform_real_distribution<float> axisDistribution(
        -config->PlayfieldHalfExtent * 0.92f,
        config->PlayfieldHalfExtent * 0.92f
    );

    return Vector3(axisDistribution(world.Random), 0.0f, axisDistribution(world.Random));
}

bool KatamariSpawner::IsSpawnPositionTooClose(
    Scene const &scene,
    KatamariWorldContext const &world,
    Vector3 const &candidate,
    float minimumSeparation
) const
{
    for (auto const &entry : world.Pickups)
    {
        TransformComponent const *const transform = scene.TryGetTransformComponent(entry.first);
        if (transform == nullptr)
        {
            continue;
        }

        Vector3 delta = transform->Local.Position - candidate;
        delta.y = 0.0f;
        if (delta.LengthSquared() < minimumSeparation * minimumSeparation)
        {
            return true;
        }
    }

    return false;
}
