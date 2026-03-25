#include "Core/Gameplay/Scene.h"

#include "Core/App/AppContext.h"

Entity Scene::CreateEntity()
{
    std::size_t slotIndex = 0u;
    if (!m_freeSlotIndices.empty())
    {
        slotIndex = m_freeSlotIndices.back();
        m_freeSlotIndices.pop_back();
    }
    else
    {
        slotIndex = m_slots.size();
        m_slots.emplace_back();
    }

    EntitySlot &slot = m_slots[slotIndex];
    slot = EntitySlot{};
    slot.Id = m_nextEntityId++;
    slot.Active = true;
    m_indexById[slot.Id] = slotIndex;
    return Entity(slot.Id, this);
}

void Scene::DestroyEntity(const EntityId entityId)
{
    const auto iterator = m_indexById.find(entityId);
    if (iterator == m_indexById.end())
    {
        return;
    }

    const std::size_t slotIndex = iterator->second;
    if (slotIndex >= m_slots.size())
    {
        return;
    }

    EntitySlot &slot = m_slots[slotIndex];
    slot = EntitySlot{};
    slot.Active = false;
    m_indexById.erase(iterator);
    m_freeSlotIndices.push_back(slotIndex);
}

void Scene::DestroyEntity(const Entity &entity)
{
    DestroyEntity(entity.GetId());
}

Entity Scene::GetEntityById(const EntityId entityId) const
{
    const auto iterator = m_indexById.find(entityId);
    if (iterator == m_indexById.end())
    {
        return Entity();
    }

    return Entity(entityId, const_cast<Scene *>(this));
}

bool Scene::IsEntityAlive(const EntityId entityId) const
{
    const auto iterator = m_indexById.find(entityId);
    if (iterator == m_indexById.end())
    {
        return false;
    }

    const std::size_t slotIndex = iterator->second;
    if (slotIndex >= m_slots.size())
    {
        return false;
    }

    const EntitySlot &slot = m_slots[slotIndex];
    return slot.Active && slot.Id == entityId;
}

void Scene::AddSystem(std::unique_ptr<ISceneSystem> system)
{
    if (system == nullptr)
    {
        return;
    }

    m_systems.push_back(std::move(system));
}

void Scene::ClearSystems()
{
    m_systems.clear();
}

void Scene::InitializeSystems(AppContext &context)
{
    for (std::unique_ptr<ISceneSystem> &system : m_systems)
    {
        if (system == nullptr)
        {
            continue;
        }

        system->Initialize(*this, context);
    }
}

void Scene::Update(AppContext &context, const float deltaTime)
{
    for (std::unique_ptr<ISceneSystem> &system : m_systems)
    {
        if (system == nullptr)
        {
            continue;
        }

        system->Update(*this, context, deltaTime);
    }
}

void Scene::Render(AppContext &context)
{
    for (std::unique_ptr<ISceneSystem> &system : m_systems)
    {
        if (system == nullptr)
        {
            continue;
        }

        system->Render(*this, context);
    }
}

void Scene::SetActiveCamera(Camera *const camera)
{
    m_activeCamera = camera;
}

Camera *Scene::GetActiveCamera() const
{
    return m_activeCamera;
}

void Scene::SetCollisionWorldBounds(const AxisAlignedBox3D &worldBounds)
{
    m_collisionWorldBounds = worldBounds;
}

void Scene::SetCollisionCellSize(const float cellSize)
{
    m_collisionCellSize = cellSize;
}

const AxisAlignedBox3D &Scene::GetCollisionWorldBounds() const
{
    return m_collisionWorldBounds;
}

float Scene::GetCollisionCellSize() const
{
    return m_collisionCellSize;
}

void Scene::ReplaceCollisionFrameResults(std::vector<CollisionPair> pairs)
{
    m_collisionFrameResults = std::move(pairs);
}

const std::vector<CollisionPair> &Scene::GetCollisionFrameResults() const
{
    return m_collisionFrameResults;
}

void Scene::ForEachEntityWithTransform(const std::function<void(Entity &)> &callback)
{
    for (EntitySlot &slot : m_slots)
    {
        if (!slot.Active)
        {
            continue;
        }

        if (!slot.Transform.has_value())
        {
            continue;
        }

        Entity entity(slot.Id, this);
        callback(entity);
    }
}

void Scene::ForEachEntityWithTransformAndModel(const std::function<void(Entity &)> &callback)
{
    for (EntitySlot &slot : m_slots)
    {
        if (!slot.Active)
        {
            continue;
        }

        if (!slot.Transform.has_value() || !slot.Model.has_value())
        {
            continue;
        }

        Entity entity(slot.Id, this);
        callback(entity);
    }
}

void Scene::ForEachEntityWithTransformAndSphereCollider(const std::function<void(Entity &)> &callback)
{
    for (EntitySlot &slot : m_slots)
    {
        if (!slot.Active)
        {
            continue;
        }

        if (!slot.Transform.has_value() || !slot.SphereCollider.has_value())
        {
            continue;
        }

        Entity entity(slot.Id, this);
        callback(entity);
    }
}

std::optional<std::size_t> Scene::TryFindSlotIndex(const EntityId entityId) const
{
    const auto iterator = m_indexById.find(entityId);
    if (iterator == m_indexById.end())
    {
        return std::nullopt;
    }

    return iterator->second;
}

TransformComponent *Scene::TryGetTransformComponent(const EntityId entityId)
{
    const std::optional<std::size_t> slotIndex = TryFindSlotIndex(entityId);
    if (!slotIndex.has_value())
    {
        return nullptr;
    }

    EntitySlot &slot = m_slots[*slotIndex];
    if (!slot.Active)
    {
        return nullptr;
    }

    if (!slot.Transform.has_value())
    {
        return nullptr;
    }

    return &(*slot.Transform);
}

const TransformComponent *Scene::TryGetTransformComponent(const EntityId entityId) const
{
    const std::optional<std::size_t> slotIndex = TryFindSlotIndex(entityId);
    if (!slotIndex.has_value())
    {
        return nullptr;
    }

    const EntitySlot &slot = m_slots[*slotIndex];
    if (!slot.Active)
    {
        return nullptr;
    }

    if (!slot.Transform.has_value())
    {
        return nullptr;
    }

    return &(*slot.Transform);
}

bool Scene::AddTransformComponent(const EntityId entityId, const TransformComponent &component)
{
    const std::optional<std::size_t> slotIndex = TryFindSlotIndex(entityId);
    if (!slotIndex.has_value())
    {
        return false;
    }

    EntitySlot &slot = m_slots[*slotIndex];
    if (!slot.Active)
    {
        return false;
    }

    slot.Transform = component;
    return true;
}

bool Scene::RemoveTransformComponent(const EntityId entityId)
{
    const std::optional<std::size_t> slotIndex = TryFindSlotIndex(entityId);
    if (!slotIndex.has_value())
    {
        return false;
    }

    EntitySlot &slot = m_slots[*slotIndex];
    if (!slot.Active)
    {
        return false;
    }

    slot.Transform.reset();
    return true;
}

ModelComponent *Scene::TryGetModelComponent(const EntityId entityId)
{
    const std::optional<std::size_t> slotIndex = TryFindSlotIndex(entityId);
    if (!slotIndex.has_value())
    {
        return nullptr;
    }

    EntitySlot &slot = m_slots[*slotIndex];
    if (!slot.Active)
    {
        return nullptr;
    }

    if (!slot.Model.has_value())
    {
        return nullptr;
    }

    return &(*slot.Model);
}

const ModelComponent *Scene::TryGetModelComponent(const EntityId entityId) const
{
    const std::optional<std::size_t> slotIndex = TryFindSlotIndex(entityId);
    if (!slotIndex.has_value())
    {
        return nullptr;
    }

    const EntitySlot &slot = m_slots[*slotIndex];
    if (!slot.Active)
    {
        return nullptr;
    }

    if (!slot.Model.has_value())
    {
        return nullptr;
    }

    return &(*slot.Model);
}

bool Scene::AddModelComponent(const EntityId entityId, const ModelComponent &component)
{
    const std::optional<std::size_t> slotIndex = TryFindSlotIndex(entityId);
    if (!slotIndex.has_value())
    {
        return false;
    }

    EntitySlot &slot = m_slots[*slotIndex];
    if (!slot.Active)
    {
        return false;
    }

    slot.Model = component;
    return true;
}

bool Scene::RemoveModelComponent(const EntityId entityId)
{
    const std::optional<std::size_t> slotIndex = TryFindSlotIndex(entityId);
    if (!slotIndex.has_value())
    {
        return false;
    }

    EntitySlot &slot = m_slots[*slotIndex];
    if (!slot.Active)
    {
        return false;
    }

    slot.Model.reset();
    return true;
}

SphereColliderComponent *Scene::TryGetSphereColliderComponent(const EntityId entityId)
{
    const std::optional<std::size_t> slotIndex = TryFindSlotIndex(entityId);
    if (!slotIndex.has_value())
    {
        return nullptr;
    }

    EntitySlot &slot = m_slots[*slotIndex];
    if (!slot.Active)
    {
        return nullptr;
    }

    if (!slot.SphereCollider.has_value())
    {
        return nullptr;
    }

    return &(*slot.SphereCollider);
}

const SphereColliderComponent *Scene::TryGetSphereColliderComponent(const EntityId entityId) const
{
    const std::optional<std::size_t> slotIndex = TryFindSlotIndex(entityId);
    if (!slotIndex.has_value())
    {
        return nullptr;
    }

    const EntitySlot &slot = m_slots[*slotIndex];
    if (!slot.Active)
    {
        return nullptr;
    }

    if (!slot.SphereCollider.has_value())
    {
        return nullptr;
    }

    return &(*slot.SphereCollider);
}

bool Scene::AddSphereColliderComponent(const EntityId entityId, const SphereColliderComponent &component)
{
    const std::optional<std::size_t> slotIndex = TryFindSlotIndex(entityId);
    if (!slotIndex.has_value())
    {
        return false;
    }

    EntitySlot &slot = m_slots[*slotIndex];
    if (!slot.Active)
    {
        return false;
    }

    slot.SphereCollider = component;
    return true;
}

bool Scene::RemoveSphereColliderComponent(const EntityId entityId)
{
    const std::optional<std::size_t> slotIndex = TryFindSlotIndex(entityId);
    if (!slotIndex.has_value())
    {
        return false;
    }

    EntitySlot &slot = m_slots[*slotIndex];
    if (!slot.Active)
    {
        return false;
    }

    slot.SphereCollider.reset();
    return true;
}

TagComponent *Scene::TryGetTagComponent(const EntityId entityId)
{
    const std::optional<std::size_t> slotIndex = TryFindSlotIndex(entityId);
    if (!slotIndex.has_value())
    {
        return nullptr;
    }

    EntitySlot &slot = m_slots[*slotIndex];
    if (!slot.Active)
    {
        return nullptr;
    }

    if (!slot.Tag.has_value())
    {
        return nullptr;
    }

    return &(*slot.Tag);
}

const TagComponent *Scene::TryGetTagComponent(const EntityId entityId) const
{
    const std::optional<std::size_t> slotIndex = TryFindSlotIndex(entityId);
    if (!slotIndex.has_value())
    {
        return nullptr;
    }

    const EntitySlot &slot = m_slots[*slotIndex];
    if (!slot.Active)
    {
        return nullptr;
    }

    if (!slot.Tag.has_value())
    {
        return nullptr;
    }

    return &(*slot.Tag);
}

bool Scene::AddTagComponent(const EntityId entityId, const TagComponent &component)
{
    const std::optional<std::size_t> slotIndex = TryFindSlotIndex(entityId);
    if (!slotIndex.has_value())
    {
        return false;
    }

    EntitySlot &slot = m_slots[*slotIndex];
    if (!slot.Active)
    {
        return false;
    }

    slot.Tag = component;
    return true;
}

bool Scene::RemoveTagComponent(const EntityId entityId)
{
    const std::optional<std::size_t> slotIndex = TryFindSlotIndex(entityId);
    if (!slotIndex.has_value())
    {
        return false;
    }

    EntitySlot &slot = m_slots[*slotIndex];
    if (!slot.Active)
    {
        return false;
    }

    slot.Tag.reset();
    return true;
}

VelocityComponent *Scene::TryGetVelocityComponent(const EntityId entityId)
{
    const std::optional<std::size_t> slotIndex = TryFindSlotIndex(entityId);
    if (!slotIndex.has_value())
    {
        return nullptr;
    }

    EntitySlot &slot = m_slots[*slotIndex];
    if (!slot.Active)
    {
        return nullptr;
    }

    if (!slot.Velocity.has_value())
    {
        return nullptr;
    }

    return &(*slot.Velocity);
}

const VelocityComponent *Scene::TryGetVelocityComponent(const EntityId entityId) const
{
    const std::optional<std::size_t> slotIndex = TryFindSlotIndex(entityId);
    if (!slotIndex.has_value())
    {
        return nullptr;
    }

    const EntitySlot &slot = m_slots[*slotIndex];
    if (!slot.Active)
    {
        return nullptr;
    }

    if (!slot.Velocity.has_value())
    {
        return nullptr;
    }

    return &(*slot.Velocity);
}

bool Scene::AddVelocityComponent(const EntityId entityId, const VelocityComponent &component)
{
    const std::optional<std::size_t> slotIndex = TryFindSlotIndex(entityId);
    if (!slotIndex.has_value())
    {
        return false;
    }

    EntitySlot &slot = m_slots[*slotIndex];
    if (!slot.Active)
    {
        return false;
    }

    slot.Velocity = component;
    return true;
}

bool Scene::RemoveVelocityComponent(const EntityId entityId)
{
    const std::optional<std::size_t> slotIndex = TryFindSlotIndex(entityId);
    if (!slotIndex.has_value())
    {
        return false;
    }

    EntitySlot &slot = m_slots[*slotIndex];
    if (!slot.Active)
    {
        return false;
    }

    slot.Velocity.reset();
    return true;
}

AttachmentComponent *Scene::TryGetAttachmentComponent(const EntityId entityId)
{
    const std::optional<std::size_t> slotIndex = TryFindSlotIndex(entityId);
    if (!slotIndex.has_value())
    {
        return nullptr;
    }

    EntitySlot &slot = m_slots[*slotIndex];
    if (!slot.Active)
    {
        return nullptr;
    }

    if (!slot.Attachment.has_value())
    {
        return nullptr;
    }

    return &(*slot.Attachment);
}

const AttachmentComponent *Scene::TryGetAttachmentComponent(const EntityId entityId) const
{
    const std::optional<std::size_t> slotIndex = TryFindSlotIndex(entityId);
    if (!slotIndex.has_value())
    {
        return nullptr;
    }

    const EntitySlot &slot = m_slots[*slotIndex];
    if (!slot.Active)
    {
        return nullptr;
    }

    if (!slot.Attachment.has_value())
    {
        return nullptr;
    }

    return &(*slot.Attachment);
}

bool Scene::AddAttachmentComponent(const EntityId entityId, const AttachmentComponent &component)
{
    const std::optional<std::size_t> slotIndex = TryFindSlotIndex(entityId);
    if (!slotIndex.has_value())
    {
        return false;
    }

    EntitySlot &slot = m_slots[*slotIndex];
    if (!slot.Active)
    {
        return false;
    }

    slot.Attachment = component;
    return true;
}

bool Scene::RemoveAttachmentComponent(const EntityId entityId)
{
    const std::optional<std::size_t> slotIndex = TryFindSlotIndex(entityId);
    if (!slotIndex.has_value())
    {
        return false;
    }

    EntitySlot &slot = m_slots[*slotIndex];
    if (!slot.Active)
    {
        return false;
    }

    slot.Attachment.reset();
    return true;
}
