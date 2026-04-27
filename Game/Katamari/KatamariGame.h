#ifndef PINGPONG_KATAMARIGAME_H
#define PINGPONG_KATAMARIGAME_H

#include "Core/App/IGame.h"

#include "Core/Graphics/ModelAsset.h"
#include "Core/Gameplay/Scene.h"
#include "Core/Graphics/FollowCamera.h"
#include "Core/Graphics/Rendering/Lighting/SceneLighting3D.h"
#include "Game/Katamari/Data/KatamariGameConfig.h"
#include "Game/Katamari/Data/KatamariWorldContext.h"
#include "Game/Katamari/Data/PickupArchetype.h"
#include "Game/Katamari/Systems/KatamariCameraController.h"

#include <vector>

class KatamariGame final : public IGame
{
public:
    void Initialize(AppContext &context) override;

    void Update(AppContext &context, float deltaTime) override;

    void Render(AppContext &context) override;

    void Shutdown(AppContext &context) override;

private:
    void RegisterSceneSystems(AppContext &context);

    void ConfigureParticleEmitter(AppContext &context);

    void ResetLevel(AppContext &context);

    void DestroyAllPickupEntities();

    void UpdateDebugInput(AppContext &context);

    Scene SceneInstance{};
    FollowCamera FollowCameraInstance{};
    KatamariCameraController CameraController{};
    KatamariGameConfig GameConfig{};
    KatamariWorldContext WorldContext{};
    std::vector<PickupArchetype> PickupArchetypes{};
    bool Initialized{false};
    bool RenderPipelineConfigured{false};
    bool StaticWorldCreated{false};
    float LastDeltaTime{0.0f};
    float FpsAccumulator{0.0f};
    int FpsFrames{0};
    int DisplayFps{0};
    std::shared_ptr<ModelAsset> GroundModel{};
    float GroundModelVerticalOffset{0.0f};
    std::shared_ptr<ModelAsset> MoonModel{};
    std::shared_ptr<ModelAsset> PlayerBallMeshModel{};
    SceneLightingDescriptor3D KatamariLighting{};
};

#endif
