#include "Core/Gameplay/TransformSystem.h"

#include "Core/App/AppContext.h"
#include "Core/Gameplay/Scene.h"

#include <algorithm>

int TransformSystem::ResolveAttachmentDepth(
    const Scene &scene,
    const EntityId entityId,
    std::unordered_map<EntityId, int> &depthById,
    std::unordered_set<EntityId> &visiting
)
{
    if (entityId == 0u)
    {
        return 0;
    }

    const auto memoIterator = depthById.find(entityId);
    if (memoIterator != depthById.end())
    {
        return memoIterator->second;
    }

    if (visiting.count(entityId) != 0u)
    {
        depthById[entityId] = 0;
        return 0;
    }

    visiting.insert(entityId);

    const std::optional<std::size_t> slotIndex = scene.TryFindSlotIndex(entityId);
    if (!slotIndex.has_value())
    {
        visiting.erase(entityId);
        depthById[entityId] = 0;
        return 0;
    }

    const Scene::EntitySlot &slot = scene.m_slots[*slotIndex];
    if (!slot.Active || !slot.Transform.has_value())
    {
        visiting.erase(entityId);
        depthById[entityId] = 0;
        return 0;
    }

    int depth = 0;
    if (slot.Attachment.has_value() && slot.Attachment->ParentEntityId != 0u)
    {
        depth = 1 + ResolveAttachmentDepth(scene, slot.Attachment->ParentEntityId, depthById, visiting);
    }

    visiting.erase(entityId);
    depthById[entityId] = depth;
    return depth;
}

void TransformSystem::Initialize(Scene &, AppContext &)
{
}

void TransformSystem::Update(Scene &scene, AppContext &, float)
{
    std::vector<std::size_t> indicesWithTransform{};
    indicesWithTransform.reserve(scene.m_slots.size());

    for (std::size_t index = 0; index < scene.m_slots.size(); ++index)
    {
        const Scene::EntitySlot &slot = scene.m_slots[index];
        if (!slot.Active)
        {
            continue;
        }

        if (!slot.Transform.has_value())
        {
            continue;
        }

        indicesWithTransform.push_back(index);
    }

    std::unordered_map<EntityId, int> depthById{};
    std::unordered_set<EntityId> visiting{};

    for (const std::size_t slotIndex : indicesWithTransform)
    {
        const EntityId entityId = scene.m_slots[slotIndex].Id;
        ResolveAttachmentDepth(scene, entityId, depthById, visiting);
    }

    std::sort(
        indicesWithTransform.begin(),
        indicesWithTransform.end(),
        [&](const std::size_t leftIndex, const std::size_t rightIndex) {
            const int depthLeft = depthById[scene.m_slots[leftIndex].Id];
            const int depthRight = depthById[scene.m_slots[rightIndex].Id];
            if (depthLeft != depthRight)
            {
                return depthLeft < depthRight;
            }

            return scene.m_slots[leftIndex].Id < scene.m_slots[rightIndex].Id;
        }
    );

    for (const std::size_t slotIndex : indicesWithTransform)
    {
        Scene::EntitySlot &slot = scene.m_slots[slotIndex];
        TransformComponent &transform = *slot.Transform;

        if (!slot.Attachment.has_value() || slot.Attachment->ParentEntityId == 0u)
        {
            transform.WorldMatrix = transform.Local.GetWorldMatrix();
            continue;
        }

        const EntityId parentEntityId = slot.Attachment->ParentEntityId;
        const std::optional<std::size_t> parentSlotIndex = scene.TryFindSlotIndex(parentEntityId);
        if (!parentSlotIndex.has_value())
        {
            transform.WorldMatrix = transform.Local.GetWorldMatrix();
            continue;
        }

        Scene::EntitySlot &parentSlot = scene.m_slots[*parentSlotIndex];
        if (!parentSlot.Active || !parentSlot.Transform.has_value())
        {
            transform.WorldMatrix = transform.Local.GetWorldMatrix();
            continue;
        }

        const DirectX::SimpleMath::Matrix &parentWorldMatrix = parentSlot.Transform->WorldMatrix;
        const DirectX::SimpleMath::Matrix offsetMatrix = DirectX::SimpleMath::Matrix::CreateTranslation(
            slot.Attachment->LocalOffset
        );
        transform.WorldMatrix = parentWorldMatrix * offsetMatrix * transform.Local.GetWorldMatrix();
    }
}
