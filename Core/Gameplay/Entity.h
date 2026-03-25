#ifndef PINGPONG_ENTITY_H
#define PINGPONG_ENTITY_H

#include "Core/Gameplay/EntityId.h"
#include "Core/Gameplay/GameplayComponents.h"

#include <optional>

class Scene;

class Entity final
{
public:
    Entity() = default;

    [[nodiscard]] EntityId GetId() const;

    [[nodiscard]] Scene *GetScene() const;

    [[nodiscard]] bool IsValid() const;

    [[nodiscard]] bool HasTransformComponent() const;

    [[nodiscard]] TransformComponent *TryGetTransformComponent();

    [[nodiscard]] const TransformComponent *TryGetTransformComponent() const;

    bool AddTransformComponent(const TransformComponent &component);

    bool RemoveTransformComponent();

    [[nodiscard]] bool HasModelComponent() const;

    [[nodiscard]] ModelComponent *TryGetModelComponent();

    [[nodiscard]] const ModelComponent *TryGetModelComponent() const;

    bool AddModelComponent(const ModelComponent &component);

    bool RemoveModelComponent();

    [[nodiscard]] bool HasSphereColliderComponent() const;

    [[nodiscard]] SphereColliderComponent *TryGetSphereColliderComponent();

    [[nodiscard]] const SphereColliderComponent *TryGetSphereColliderComponent() const;

    bool AddSphereColliderComponent(const SphereColliderComponent &component);

    bool RemoveSphereColliderComponent();

    [[nodiscard]] bool HasTagComponent() const;

    [[nodiscard]] TagComponent *TryGetTagComponent();

    [[nodiscard]] const TagComponent *TryGetTagComponent() const;

    bool AddTagComponent(const TagComponent &component);

    bool RemoveTagComponent();

    [[nodiscard]] bool HasVelocityComponent() const;

    [[nodiscard]] VelocityComponent *TryGetVelocityComponent();

    [[nodiscard]] const VelocityComponent *TryGetVelocityComponent() const;

    bool AddVelocityComponent(const VelocityComponent &component);

    bool RemoveVelocityComponent();

    [[nodiscard]] bool HasAttachmentComponent() const;

    [[nodiscard]] AttachmentComponent *TryGetAttachmentComponent();

    [[nodiscard]] const AttachmentComponent *TryGetAttachmentComponent() const;

    bool AddAttachmentComponent(const AttachmentComponent &component);

    bool RemoveAttachmentComponent();

private:
    friend class Scene;

    Entity(EntityId entityId, Scene *ownerScene);

    EntityId m_entityId{0u};
    Scene *m_ownerScene{nullptr};
};

#endif
