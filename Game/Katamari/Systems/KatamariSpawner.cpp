#include "Game/Katamari/Systems/KatamariSpawner.h"

#include "Core/App/AppContext.h"
#include "Core/Assets/AssetCache.h"
#include "Core/Gameplay/Entity.h"
#include "Core/Gameplay/GameplayComponents.h"
#include "Core/Gameplay/Scene.h"
#include "Core/Graphics/ModelAsset.h"
#include "Game/Katamari/KatamariTypes.h"

#include <cmath>
#include <random>

using DirectX::SimpleMath::Vector3;

namespace
{
[[nodiscard]] float RadiusFromMergedModelBounds(
    std::shared_ptr<ModelAsset> const &modelAsset,
    float visualUniformScale,
    float archetypeRadiusFallback
)
{
    const float minimumRadius = archetypeRadiusFallback > 0.0f ? archetypeRadiusFallback : 0.2f;

    if (modelAsset == nullptr || !modelAsset->IsLoaded())
    {
        return (std::max)(minimumRadius, 0.35f);
    }

    const DirectX::BoundingSphere merged = modelAsset->GetMergedBoundingSphere();
    const float scaled = merged.Radius * visualUniformScale;
    return (std::max)(scaled, minimumRadius);
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

        const float colliderRadius = RadiusFromMergedModelBounds(
            model,
            archetype.VisualUniformScale,
            archetype.CollisionSphereRadius
        );

        Vector3 position{};
        bool placed = false;
        for (unsigned int attempt = 0u; attempt < maxAttempts; ++attempt)
        {
            position = SampleSpawnPosition(world);
            position.y = colliderRadius;
            if (!IsSpawnPositionTooClose(scene, world, position, separation + colliderRadius))
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
        pickup.AddModelComponent(modelComponent);

        SphereColliderComponent sphere{};
        sphere.Radius = colliderRadius;
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
        record.CollisionSphereRadius = colliderRadius;
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
