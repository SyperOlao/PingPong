#ifndef PINGPONG_KATAMARIABSORPTIONSYSTEM_H
#define PINGPONG_KATAMARIABSORPTIONSYSTEM_H

#include "Core/Gameplay/ISceneSystem.h"

struct KatamariWorldContext;

class KatamariAbsorptionSystem final : public ISceneSystem
{
public:
    explicit KatamariAbsorptionSystem(KatamariWorldContext *gameplayWorld) noexcept;

    void Initialize(Scene &scene, AppContext &context) override;

    void Update(Scene &scene, AppContext &context, float deltaTime) override;

private:
    KatamariWorldContext *GameplayWorld{nullptr};
};

#endif
