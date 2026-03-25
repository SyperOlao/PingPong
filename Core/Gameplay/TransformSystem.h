#ifndef PINGPONG_TRANSFORMSYSTEM_H
#define PINGPONG_TRANSFORMSYSTEM_H

#include "Core/Gameplay/EntityId.h"
#include "Core/Gameplay/ISceneSystem.h"

#include <unordered_map>
#include <unordered_set>

class TransformSystem final : public ISceneSystem
{
public:
    void Initialize(Scene &scene, AppContext &context) override;

    void Update(Scene &scene, AppContext &context, float deltaTime) override;

private:
    static int ResolveAttachmentDepth(
        const Scene &scene,
        EntityId entityId,
        std::unordered_map<EntityId, int> &depthById,
        std::unordered_set<EntityId> &visiting
    );
};

#endif
