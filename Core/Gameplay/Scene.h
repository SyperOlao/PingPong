#ifndef PINGPONG_SCENE_H
#define PINGPONG_SCENE_H

#include "Core/Gameplay/Entity.h"
#include "Core/Gameplay/GameplayComponents.h"
#include "Core/Gameplay/ISceneSystem.h"
#include "Core/Gameplay/SceneCollisionTypes.h"
#include "Core/Physics/Collision3D/AxisAlignedBox3D.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

class Camera;
class TransformSystem;
class CollisionSystem;
class VelocityIntegrationSystem;

class Scene final
{
public:
    Scene() = default;

    Scene(const Scene &) = delete;

    Scene &operator=(const Scene &) = delete;

    Scene(Scene &&) = delete;

    Scene &operator=(Scene &&) = delete;

    ~Scene() = default;

    [[nodiscard]] Entity CreateEntity();

    void DestroyEntity(EntityId entityId);

    void DestroyEntity(const Entity &entity);

    [[nodiscard]] Entity GetEntityById(EntityId entityId) const;

    [[nodiscard]] bool IsEntityAlive(EntityId entityId) const;

    void AddSystem(std::unique_ptr<ISceneSystem> system);

    void ClearSystems();

    void InitializeSystems(AppContext &context);

    void Update(AppContext &context, float deltaTime);

    void Render(AppContext &context);

    void SetActiveCamera(Camera *camera);

    [[nodiscard]] Camera *GetActiveCamera() const;

    void SetCollisionWorldBounds(const AxisAlignedBox3D &worldBounds);

    void SetCollisionCellSize(float cellSize);

    [[nodiscard]] const AxisAlignedBox3D &GetCollisionWorldBounds() const;

    [[nodiscard]] float GetCollisionCellSize() const;

    void ReplaceCollisionFrameResults(std::vector<CollisionPair> pairs);

    [[nodiscard]] const std::vector<CollisionPair> &GetCollisionFrameResults() const;

    void ForEachEntityWithTransform(const std::function<void(Entity &)> &callback);

    void ForEachEntityWithTransformAndModel(const std::function<void(Entity &)> &callback);

    void ForEachEntityWithTransformAndSphereCollider(const std::function<void(Entity &)> &callback);

    [[nodiscard]] TransformComponent *TryGetTransformComponent(EntityId entityId);

    [[nodiscard]] const TransformComponent *TryGetTransformComponent(EntityId entityId) const;

    bool AddTransformComponent(EntityId entityId, const TransformComponent &component);

    bool RemoveTransformComponent(EntityId entityId);

    [[nodiscard]] ModelComponent *TryGetModelComponent(EntityId entityId);

    [[nodiscard]] const ModelComponent *TryGetModelComponent(EntityId entityId) const;

    bool AddModelComponent(EntityId entityId, const ModelComponent &component);

    bool RemoveModelComponent(EntityId entityId);

    [[nodiscard]] SphereColliderComponent *TryGetSphereColliderComponent(EntityId entityId);

    [[nodiscard]] const SphereColliderComponent *TryGetSphereColliderComponent(EntityId entityId) const;

    bool AddSphereColliderComponent(EntityId entityId, const SphereColliderComponent &component);

    bool RemoveSphereColliderComponent(EntityId entityId);

    [[nodiscard]] TagComponent *TryGetTagComponent(EntityId entityId);

    [[nodiscard]] const TagComponent *TryGetTagComponent(EntityId entityId) const;

    bool AddTagComponent(EntityId entityId, const TagComponent &component);

    bool RemoveTagComponent(EntityId entityId);

    [[nodiscard]] VelocityComponent *TryGetVelocityComponent(EntityId entityId);

    [[nodiscard]] const VelocityComponent *TryGetVelocityComponent(EntityId entityId) const;

    bool AddVelocityComponent(EntityId entityId, const VelocityComponent &component);

    bool RemoveVelocityComponent(EntityId entityId);

    [[nodiscard]] AttachmentComponent *TryGetAttachmentComponent(EntityId entityId);

    [[nodiscard]] const AttachmentComponent *TryGetAttachmentComponent(EntityId entityId) const;

    bool AddAttachmentComponent(EntityId entityId, const AttachmentComponent &component);

    bool RemoveAttachmentComponent(EntityId entityId);

private:
    friend class TransformSystem;
    friend class CollisionSystem;
    friend class VelocityIntegrationSystem;

    struct EntitySlot final
    {
        EntityId Id{0u};
        bool Active{false};
        std::optional<TransformComponent> Transform{};
        std::optional<ModelComponent> Model{};
        std::optional<SphereColliderComponent> SphereCollider{};
        std::optional<TagComponent> Tag{};
        std::optional<VelocityComponent> Velocity{};
        std::optional<AttachmentComponent> Attachment{};
    };

    [[nodiscard]] std::optional<std::size_t> TryFindSlotIndex(EntityId entityId) const;

    std::vector<EntitySlot> m_slots{};
    std::unordered_map<EntityId, std::size_t> m_indexById{};
    std::vector<std::size_t> m_freeSlotIndices{};
    EntityId m_nextEntityId{1u};
    std::vector<std::unique_ptr<ISceneSystem>> m_systems{};
    Camera *m_activeCamera{nullptr};
    AxisAlignedBox3D m_collisionWorldBounds = AxisAlignedBox3D{
        DirectX::SimpleMath::Vector3(-2000.0f, -2000.0f, -2000.0f),
        DirectX::SimpleMath::Vector3(2000.0f, 2000.0f, 2000.0f)
    };
    float m_collisionCellSize{4.0f};
    std::vector<CollisionPair> m_collisionFrameResults{};
};

#endif
