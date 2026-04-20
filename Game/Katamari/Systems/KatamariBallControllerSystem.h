#ifndef PINGPONG_KATAMARIBALLCONTROLLERSYSTEM_H
#define PINGPONG_KATAMARIBALLCONTROLLERSYSTEM_H

#include "Core/Gameplay/ISceneSystem.h"

struct KatamariWorldContext;

class KatamariBallControllerSystem final : public ISceneSystem
{
public:
    explicit KatamariBallControllerSystem(KatamariWorldContext *gameplayWorld) noexcept;

    void Initialize(Scene &scene, AppContext &context) override;

    void Update(Scene &scene, AppContext &context, float deltaTime) override;

private:
    KatamariWorldContext *GameplayWorld{nullptr};
};

#endif
