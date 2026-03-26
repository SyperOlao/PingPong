#ifndef PINGPONG_LIGHTINGTESTGAME_H
#define PINGPONG_LIGHTINGTESTGAME_H

#include "Core/App/IGame.h"
#include "Core/Graphics/FpsCamera.h"
#include "Core/Graphics/ModelAsset.h"
#include "Core/Graphics/Rendering/Lighting/SceneLighting3D.h"

struct AppContext;

class LightingTestGame final : public IGame
{
public:
    void Initialize(AppContext &context) override;
    void Update(AppContext &context, float deltaTime) override;
    void Render(AppContext &context) override;

private:
    void UpdateCamera(const AppContext &context, float deltaTime);

    FpsCamera SceneCamera{};
    std::shared_ptr<ModelAsset> CubeModel{};
    std::shared_ptr<ModelAsset> SphereModel{};
    SceneLightingDescriptor3D Lighting{};
    float AccumulatedTimeSeconds{0.0f};
    bool IsInitialized{false};
};

#endif
