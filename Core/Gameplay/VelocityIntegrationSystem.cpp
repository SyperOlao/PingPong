#include "Core/Gameplay/VelocityIntegrationSystem.h"

#include "Core/App/AppContext.h"
#include "Core/Gameplay/Scene.h"

void VelocityIntegrationSystem::Initialize(Scene &, AppContext &)
{
}

void VelocityIntegrationSystem::Update(Scene &scene, AppContext &, const float deltaTime)
{
    for (Scene::EntitySlot &slot : scene.m_slots)
    {
        if (!slot.Active)
        {
            continue;
        }

        if (!slot.Transform.has_value() || !slot.Velocity.has_value())
        {
            continue;
        }

        if (slot.Attachment.has_value() && slot.Attachment->ParentEntityId != 0u)
        {
            continue;
        }

        slot.Transform->Local.Position += slot.Velocity->LinearVelocity * deltaTime;
    }
}
