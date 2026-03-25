#ifndef PINGPONG_VELOCITYINTEGRATIONSYSTEM_H
#define PINGPONG_VELOCITYINTEGRATIONSYSTEM_H

#include "Core/Gameplay/ISceneSystem.h"

class VelocityIntegrationSystem final : public ISceneSystem
{
public:
    void Initialize(Scene &scene, AppContext &context) override;

    void Update(Scene &scene, AppContext &context, float deltaTime) override;
};

#endif
