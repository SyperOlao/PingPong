#ifndef PINGPONG_KATAMARISPHEREWORLDRESOLVESYSTEM_H
#define PINGPONG_KATAMARISPHEREWORLDRESOLVESYSTEM_H

#include "Core/Gameplay/ISceneSystem.h"

struct KatamariWorldContext;

class KatamariSphereWorldResolveSystem final : public ISceneSystem
{
public:
    explicit KatamariSphereWorldResolveSystem(KatamariWorldContext *gameplayWorld) noexcept;

    void Initialize(Scene &scene, AppContext &context) override;

    void Update(Scene &scene, AppContext &context, float deltaTime) override;

private:
    KatamariWorldContext *GameplayWorld{nullptr};
};

#endif
