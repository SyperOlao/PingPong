#ifndef PINGPONG_RENDERSYSTEM_H
#define PINGPONG_RENDERSYSTEM_H

#include "Core/Gameplay/ISceneSystem.h"

class RenderSystem final : public ISceneSystem
{
public:
    void Initialize(Scene &scene, AppContext &context) override;

    void Update(Scene &scene, AppContext &context, float deltaTime) override;

    void Render(Scene &scene, AppContext &context) override;
};

#endif
