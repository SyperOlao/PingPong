#ifndef PINGPONG_KATAMARIGAME_H
#define PINGPONG_KATAMARIGAME_H

#include "Core/App/IGame.h"

#include "Core/Graphics/ModelAsset.h"
#include "Core/Gameplay/Scene.h"
#include "Core/Graphics/Particles/GpuParticleTypes.h"
#include "Core/Graphics/FollowCamera.h"
#include "Core/Graphics/Rendering/Lighting/SceneLighting3D.h"
#include "Core/UI/Button.h"
#include "Game/Katamari/Data/KatamariGameConfig.h"
#include "Game/Katamari/Data/KatamariWorldContext.h"
#include "Game/Katamari/Data/PickupArchetype.h"
#include "Game/Katamari/Systems/KatamariCameraController.h"
#include "Game/Katamari/UI/KatamariParticleSettingsPanel.h"

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

    void InitializeParticleUi(AppContext &context);

    void UpdateParticleUi(AppContext &context);

    void RenderParticleUi(AppContext &context);

    void ApplyParticleSettingsToEmitter(AppContext &context);

    void ResetLevel(AppContext &context);

    void DestroyAllPickupEntities();

    void UpdateDebugInput(AppContext &context);

    void CreateGroundEntityIfAvailable();

    Scene SceneInstance{};
    FollowCamera FollowCameraInstance{};
    KatamariCameraController CameraController{};
    KatamariGameConfig GameConfig{};
    KatamariWorldContext WorldContext{};
    std::vector<PickupArchetype> PickupArchetypes{};
    bool Initialized{false};
    bool RenderPipelineConfigured{false};
    bool StaticWorldCreated{false};
    bool ParticleSettingsInitialized{false};
    bool PreviousParticleUiLeftMouseDown{false};
    float LastDeltaTime{0.0f};
    float FpsAccumulator{0.0f};
    int FpsFrames{0};
    int DisplayFps{0};
    std::shared_ptr<ModelAsset> GroundModel{};
    EntityId GroundEntityId{0u};
    float GroundModelVerticalOffset{0.0f};
    std::shared_ptr<ModelAsset> MoonModel{};
    std::shared_ptr<ModelAsset> PlayerBallMeshModel{};
    std::shared_ptr<ModelAsset> StaticCubeModel{};
    std::shared_ptr<ModelAsset> StaticTriangularPrismModel{};
    Button ParticleSpawnButton{};
    Button ParticleSettingsButton{};
    KatamariParticleSettingsPanel ParticleSettingsPanel{};
    GpuParticleEmitterDesc CurrentParticleEmitterDesc{};
    SceneLightingDescriptor3D KatamariLighting{};
};

#endif
