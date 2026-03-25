#include "Core/Gameplay/Entity.h"

#include "Core/Gameplay/Scene.h"

Entity::Entity(const EntityId entityId, Scene *const ownerScene) : m_entityId(entityId), m_ownerScene(ownerScene)
{
}

EntityId Entity::GetId() const
{
    return m_entityId;
}

Scene *Entity::GetScene() const
{
    return m_ownerScene;
}

bool Entity::IsValid() const
{
    if (m_entityId == 0u || m_ownerScene == nullptr)
    {
        return false;
    }

    return m_ownerScene->IsEntityAlive(m_entityId);
}

bool Entity::HasTransformComponent() const
{
    if (m_ownerScene == nullptr)
    {
        return false;
    }

    return m_ownerScene->TryGetTransformComponent(m_entityId) != nullptr;
}

TransformComponent *Entity::TryGetTransformComponent()
{
    if (m_ownerScene == nullptr)
    {
        return nullptr;
    }

    return m_ownerScene->TryGetTransformComponent(m_entityId);
}

const TransformComponent *Entity::TryGetTransformComponent() const
{
    if (m_ownerScene == nullptr)
    {
        return nullptr;
    }

    return m_ownerScene->TryGetTransformComponent(m_entityId);
}

bool Entity::AddTransformComponent(const TransformComponent &component)
{
    if (m_ownerScene == nullptr)
    {
        return false;
    }

    return m_ownerScene->AddTransformComponent(m_entityId, component);
}

bool Entity::RemoveTransformComponent()
{
    if (m_ownerScene == nullptr)
    {
        return false;
    }

    return m_ownerScene->RemoveTransformComponent(m_entityId);
}

bool Entity::HasModelComponent() const
{
    if (m_ownerScene == nullptr)
    {
        return false;
    }

    return m_ownerScene->TryGetModelComponent(m_entityId) != nullptr;
}

ModelComponent *Entity::TryGetModelComponent()
{
    if (m_ownerScene == nullptr)
    {
        return nullptr;
    }

    return m_ownerScene->TryGetModelComponent(m_entityId);
}

const ModelComponent *Entity::TryGetModelComponent() const
{
    if (m_ownerScene == nullptr)
    {
        return nullptr;
    }

    return m_ownerScene->TryGetModelComponent(m_entityId);
}

bool Entity::AddModelComponent(const ModelComponent &component)
{
    if (m_ownerScene == nullptr)
    {
        return false;
    }

    return m_ownerScene->AddModelComponent(m_entityId, component);
}

bool Entity::RemoveModelComponent()
{
    if (m_ownerScene == nullptr)
    {
        return false;
    }

    return m_ownerScene->RemoveModelComponent(m_entityId);
}

bool Entity::HasSphereColliderComponent() const
{
    if (m_ownerScene == nullptr)
    {
        return false;
    }

    return m_ownerScene->TryGetSphereColliderComponent(m_entityId) != nullptr;
}

SphereColliderComponent *Entity::TryGetSphereColliderComponent()
{
    if (m_ownerScene == nullptr)
    {
        return nullptr;
    }

    return m_ownerScene->TryGetSphereColliderComponent(m_entityId);
}

const SphereColliderComponent *Entity::TryGetSphereColliderComponent() const
{
    if (m_ownerScene == nullptr)
    {
        return nullptr;
    }

    return m_ownerScene->TryGetSphereColliderComponent(m_entityId);
}

bool Entity::AddSphereColliderComponent(const SphereColliderComponent &component)
{
    if (m_ownerScene == nullptr)
    {
        return false;
    }

    return m_ownerScene->AddSphereColliderComponent(m_entityId, component);
}

bool Entity::RemoveSphereColliderComponent()
{
    if (m_ownerScene == nullptr)
    {
        return false;
    }

    return m_ownerScene->RemoveSphereColliderComponent(m_entityId);
}

bool Entity::HasTagComponent() const
{
    if (m_ownerScene == nullptr)
    {
        return false;
    }

    return m_ownerScene->TryGetTagComponent(m_entityId) != nullptr;
}

TagComponent *Entity::TryGetTagComponent()
{
    if (m_ownerScene == nullptr)
    {
        return nullptr;
    }

    return m_ownerScene->TryGetTagComponent(m_entityId);
}

const TagComponent *Entity::TryGetTagComponent() const
{
    if (m_ownerScene == nullptr)
    {
        return nullptr;
    }

    return m_ownerScene->TryGetTagComponent(m_entityId);
}

bool Entity::AddTagComponent(const TagComponent &component)
{
    if (m_ownerScene == nullptr)
    {
        return false;
    }

    return m_ownerScene->AddTagComponent(m_entityId, component);
}

bool Entity::RemoveTagComponent()
{
    if (m_ownerScene == nullptr)
    {
        return false;
    }

    return m_ownerScene->RemoveTagComponent(m_entityId);
}

bool Entity::HasVelocityComponent() const
{
    if (m_ownerScene == nullptr)
    {
        return false;
    }

    return m_ownerScene->TryGetVelocityComponent(m_entityId) != nullptr;
}

VelocityComponent *Entity::TryGetVelocityComponent()
{
    if (m_ownerScene == nullptr)
    {
        return nullptr;
    }

    return m_ownerScene->TryGetVelocityComponent(m_entityId);
}

const VelocityComponent *Entity::TryGetVelocityComponent() const
{
    if (m_ownerScene == nullptr)
    {
        return nullptr;
    }

    return m_ownerScene->TryGetVelocityComponent(m_entityId);
}

bool Entity::AddVelocityComponent(const VelocityComponent &component)
{
    if (m_ownerScene == nullptr)
    {
        return false;
    }

    return m_ownerScene->AddVelocityComponent(m_entityId, component);
}

bool Entity::RemoveVelocityComponent()
{
    if (m_ownerScene == nullptr)
    {
        return false;
    }

    return m_ownerScene->RemoveVelocityComponent(m_entityId);
}

bool Entity::HasAttachmentComponent() const
{
    if (m_ownerScene == nullptr)
    {
        return false;
    }

    return m_ownerScene->TryGetAttachmentComponent(m_entityId) != nullptr;
}

AttachmentComponent *Entity::TryGetAttachmentComponent()
{
    if (m_ownerScene == nullptr)
    {
        return nullptr;
    }

    return m_ownerScene->TryGetAttachmentComponent(m_entityId);
}

const AttachmentComponent *Entity::TryGetAttachmentComponent() const
{
    if (m_ownerScene == nullptr)
    {
        return nullptr;
    }

    return m_ownerScene->TryGetAttachmentComponent(m_entityId);
}

bool Entity::AddAttachmentComponent(const AttachmentComponent &component)
{
    if (m_ownerScene == nullptr)
    {
        return false;
    }

    return m_ownerScene->AddAttachmentComponent(m_entityId, component);
}

bool Entity::RemoveAttachmentComponent()
{
    if (m_ownerScene == nullptr)
    {
        return false;
    }

    return m_ownerScene->RemoveAttachmentComponent(m_entityId);
}
