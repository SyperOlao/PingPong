#include "Game/Katamari/KatamariGame.h"

#include "Core/App/AppContext.h"
#include "Core/Assets/AssetCache.h"
#include "Core/Gameplay/CollisionDebugDraw.h"
#include "Core/Gameplay/CollisionSystem.h"
#include "Core/Gameplay/RenderSystem.h"
#include "Core/Gameplay/TransformSystem.h"
#include "Core/Gameplay/VelocityIntegrationSystem.h"
#include "Core/Graphics/ModelRenderer.h"
#include "Core/Graphics/Particles/GpuParticleSystem.h"
#include "Core/Graphics/Rendering/Lighting/SceneLighting3D.h"
#include "Core/Graphics/Rendering/RenderContext.h"
#include "Core/Graphics/Rendering/Renderables/RenderMaterialParameters.h"
#include "Core/Input/InputSystem.h"
#include "Core/Input/Keyboard.h"
#include "Core/Input/RawInputHandler.h"
#include "Game/Katamari/Data/KatamariPickupCatalog.h"
#include "Game/Katamari/KatamariLevelSetup.h"
#include "Game/Katamari/KatamariSceneLighting.h"
#include "Game/Katamari/Systems/KatamariAbsorptionSystem.h"
#include "Game/Katamari/Systems/KatamariBallControllerSystem.h"
#include "Game/Katamari/Systems/KatamariBallRollSystem.h"
#include "Game/Katamari/Systems/KatamariBallVisualRadiusSystem.h"
#include "Game/Katamari/Systems/KatamariSphereWorldResolveSystem.h"
#include "Game/Katamari/Systems/KatamariStaticObstacleRenderSystem.h"
#include "Game/Katamari/UI/KatamariHud.h"
#include "Core/Platform/Window.h"
#include "Core/Graphics/Rendering/PrimitiveRenderer3D.h"

#include <DirectXCollision.h>
#include <algorithm>
#include <cmath>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <array>

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

namespace
{
constexpr std::array<Vector3, 16> NightStarPositions{
    Vector3(-0.88f, 0.38f, 0.28f), Vector3(-0.72f, 0.48f, -0.5f), Vector3(-0.54f, 0.44f, 0.68f),
    Vector3(-0.32f, 0.42f, -0.72f), Vector3(-0.12f, 0.5f, 0.78f), Vector3(0.08f, 0.37f, -0.84f),
    Vector3(0.28f, 0.52f, 0.64f), Vector3(0.44f, 0.4f, -0.58f), Vector3(0.62f, 0.46f, 0.48f),
    Vector3(0.76f, 0.43f, -0.34f), Vector3(0.9f, 0.36f, 0.12f), Vector3(-0.66f, 0.54f, 0.06f),
    Vector3(-0.4f, 0.5f, -0.14f), Vector3(-0.06f, 0.58f, -0.02f), Vector3(0.22f, 0.56f, -0.22f),
    Vector3(0.52f, 0.55f, 0.18f)
};

POINT GetMouseClientPosition(HWND__ *const hwnd)
{
    POINT point{};
    GetCursorPos(&point);
    ScreenToClient(hwnd, &point);
    return point;
}

DirectX::BoundingBox ComputeMergedModelBoundingBox(std::shared_ptr<ModelAsset> const &modelAsset)
{
    DirectX::BoundingBox mergedBounds{};
    if (modelAsset == nullptr || !modelAsset->IsLoaded() || modelAsset->Get() == nullptr || modelAsset->Get()->meshes.empty())
    {
        return mergedBounds;
    }

    mergedBounds = modelAsset->Get()->meshes[0]->boundingBox;
    for (size_t meshIndex = 1; meshIndex < modelAsset->Get()->meshes.size(); ++meshIndex)
    {
        DirectX::BoundingBox::CreateMerged(mergedBounds, mergedBounds, modelAsset->Get()->meshes[meshIndex]->boundingBox);
    }

    return mergedBounds;
}
}

void KatamariGame::Initialize(AppContext &context)
{
    if (!context.IsValid())
    {
        throw std::runtime_error("KatamariGame::Initialize received invalid AppContext.");
    }

    if (context.Graphics.Render != nullptr && !RenderPipelineConfigured)
    {
        context.Graphics.Render->SetRenderMode(RenderMode::Deferred);
        context.Graphics.Render->SetGameRenderCallbackForUserInterfacePassEnabled(true);
        RenderPipelineConfigured = true;
    }
    ConfigureParticleEmitter(context);
    InitializeParticleUi(context);

    PickupArchetypes = KatamariPickupCatalog::BuildDefaultPickupArchetypes();

    SceneInstance.SetForwardLightingEnabled(true);
    SceneInstance.SetDirectionalShadowMappingEnabled(true);
    SceneInstance.SetActiveCamera(&FollowCameraInstance);

    KatamariLighting = CreateKatamariSceneLighting(GameConfig);
    SceneInstance.GetSceneLightingDescriptor() = KatamariLighting;

    FollowCameraInstance.SetNearPlaneAndFarPlane(0.28f, 260.0f);
    FollowCameraInstance.SetOffsetFromTarget(Vector3(0.0f, 9.0f, -26.0f));
    FollowCameraInstance.SetPositionLagSeconds(0.14f);
    FollowCameraInstance.SetOrientationLagSeconds(0.2f);
    FollowCameraInstance.SetZoomFromRadiusParameters(GameConfig.InitialBallRadius, 0.7f, 2.2f);
    CameraController.Initialize(FollowCameraInstance);

    RegisterSceneSystems(context);

    if (AssetCache *const assetCache = context.Assets.Cache; assetCache != nullptr)
    {
        GroundModel = assetCache->LoadModel("Game/Katamari/Assets/Environment/ground.obj");
        MoonModel = assetCache->LoadModel("Game/Katamari/Assets/Environment/moon_sphere.obj");
        PlayerBallMeshModel = assetCache->LoadModel("Game/Katamari/Assets/Environment/moon_sphere.obj");
        StaticCubeModel = assetCache->LoadModel("Game/Katamari/Assets/Environment/unit_cube.obj");
        StaticTriangularPrismModel = assetCache->LoadModel("Game/Katamari/Assets/Environment/unit_triangular_prism.obj");
    }

    GroundModelVerticalOffset = 0.0f;
    if (GroundModel != nullptr && GroundModel->IsLoaded() && GroundModel->Get() != nullptr)
    {
        const DirectX::BoundingBox groundBounds = ComputeMergedModelBoundingBox(GroundModel);
        const float groundTopLocalY = groundBounds.Center.y + groundBounds.Extents.y;
        GroundModelVerticalOffset = -groundTopLocalY;
    }
    CreateGroundEntityIfAvailable();

    if (!StaticWorldCreated)
    {
        KatamariLevelSetup::CreateStaticWorldCollision(SceneInstance, GameConfig);
        KatamariLevelSetup::CreateStaticObstacles(
            SceneInstance,
            WorldContext,
            GameConfig,
            StaticCubeModel,
            StaticTriangularPrismModel
        );
        StaticWorldCreated = true;
    }

    KatamariLevelSetup::SpawnPlayerBallAndPickups(
        SceneInstance,
        context,
        WorldContext,
        GameConfig,
        PickupArchetypes,
        PlayerBallMeshModel
    );
    WorldContext.FollowCameraForMovement = &FollowCameraInstance;
    WorldContext.Config = &GameConfig;

    Initialized = true;
}

void KatamariGame::RegisterSceneSystems(AppContext &context)
{
    SceneInstance.ClearSystems();
    SceneInstance.AddSystem(std::make_unique<KatamariBallControllerSystem>(&WorldContext));
    SceneInstance.AddSystem(std::make_unique<VelocityIntegrationSystem>());
    SceneInstance.AddSystem(std::make_unique<KatamariBallVisualRadiusSystem>(&WorldContext));
    SceneInstance.AddSystem(std::make_unique<TransformSystem>());
    SceneInstance.AddSystem(std::make_unique<CollisionSystem>());
    SceneInstance.AddSystem(std::make_unique<KatamariSphereWorldResolveSystem>(&WorldContext));
    SceneInstance.AddSystem(std::make_unique<KatamariAbsorptionSystem>(&WorldContext));
    SceneInstance.AddSystem(std::make_unique<KatamariBallRollSystem>(&WorldContext));
    SceneInstance.AddSystem(std::make_unique<RenderSystem>());
    SceneInstance.AddSystem(std::make_unique<KatamariStaticObstacleRenderSystem>(&WorldContext));
    SceneInstance.InitializeSystems(context);
}

void KatamariGame::CreateGroundEntityIfAvailable()
{
    if (GroundEntityId != 0u && SceneInstance.IsEntityAlive(GroundEntityId))
    {
        return;
    }

    if (GroundModel == nullptr || !GroundModel->IsLoaded())
    {
        return;
    }

    const float playfieldSpan = GameConfig.PlayfieldHalfExtent * 2.0f + GameConfig.WallThickness * 2.0f;

    Entity ground = SceneInstance.CreateEntity();
    GroundEntityId = ground.GetId();

    TransformComponent transform{};
    transform.Local.Scale = Vector3(playfieldSpan, 1.0f, playfieldSpan);
    transform.Local.Position = Vector3(0.0f, GroundModelVerticalOffset, 0.0f);
    transform.WorldMatrix = transform.Local.GetWorldMatrix();
    ground.AddTransformComponent(transform);

    ModelComponent model{};
    model.Asset = GroundModel;
    model.Visible = true;
    model.CastsShadow = false;
    ground.AddModelComponent(model);

    MaterialComponent material{};
    material.Parameters.BaseColor = DirectX::SimpleMath::Color(0.62f, 0.7f, 0.88f, 1.0f);
    material.Parameters.ReceiveLighting = true;
    material.Parameters.AmbientFactor = 0.09f;
    material.Parameters.SpecularPower = 92.0f;
    material.Parameters.SpecularColor = DirectX::SimpleMath::Color(0.72f, 0.72f, 0.72f, 1.0f);
    ground.AddMaterialComponent(material);
}

void KatamariGame::ConfigureParticleEmitter(AppContext &context)
{
    if (context.Graphics.Render == nullptr)
    {
        return;
    }

    CurrentParticleEmitterDesc = CreateKatamariParticleEmitterDesc(GameConfig);
    CurrentParticleEmitterDesc.Enabled = false;
    CurrentParticleEmitterDesc.DepthCollisionEnabled = false;
    context.Graphics.Render->GetGpuParticleSystem().SetEmitterDesc(CurrentParticleEmitterDesc);
}

void KatamariGame::InitializeParticleUi(AppContext &context)
{
    if (ParticleSettingsInitialized)
    {
        return;
    }

    if (context.Graphics.Render != nullptr)
    {
        CurrentParticleEmitterDesc = context.Graphics.Render->GetGpuParticleSystem().GetEmitterDesc();
    }
    else
    {
        CurrentParticleEmitterDesc = CreateKatamariParticleEmitterDesc(GameConfig);
        CurrentParticleEmitterDesc.Enabled = false;
        CurrentParticleEmitterDesc.DepthCollisionEnabled = false;
    }

    ParticleSettingsPanel.Initialize(CurrentParticleEmitterDesc, GameConfig.PlayfieldHalfExtent);
    ParticleSettingsInitialized = true;
}

void KatamariGame::UpdateParticleUi(AppContext &context)
{
    if (!ParticleSettingsInitialized)
    {
        InitializeParticleUi(context);
    }

    const Window *const window = context.Platform.MainWindow;
    if (window == nullptr || window->GetWidth() <= 0 || window->GetHeight() <= 0)
    {
        return;
    }

    constexpr float buttonWidth = 420.0f;
    constexpr float buttonHeight = 48.0f;
    constexpr float buttonGap = 20.0f;
    constexpr float panelWidth = 390.0f;
    const float windowWidth = static_cast<float>(window->GetWidth());
    const float windowHeight = static_cast<float>(window->GetHeight());
    constexpr float leftMargin = 20.0f;
    constexpr float rightMargin = 20.0f;
    constexpr float renderButtonWidth = 360.0f;
    const float renderButtonY = windowHeight - 68.0f;
    const float buttonRowWidth = buttonWidth * 2.0f + buttonGap;
    const float minimumWidthForSingleRow =
        leftMargin + renderButtonWidth + buttonGap + buttonRowWidth + rightMargin;
    const bool placeButtonsInRow = windowWidth >= minimumWidthForSingleRow;
    const float buttonY = placeButtonsInRow ? renderButtonY : renderButtonY - (buttonHeight + buttonGap);
    const float spawnButtonXUnclamped = placeButtonsInRow
        ? windowWidth - rightMargin - buttonRowWidth
        : windowWidth - rightMargin - buttonWidth;
    const float spawnButtonX = (std::max)(leftMargin, spawnButtonXUnclamped);
    const float settingsButtonX = placeButtonsInRow ? spawnButtonX + buttonWidth + buttonGap : spawnButtonX;
    const float settingsButtonY = placeButtonsInRow ? buttonY : renderButtonY;
    const float panelX = (std::min)(
        settingsButtonX,
        (std::max)(20.0f, windowWidth - panelWidth - 20.0f)
    );
    ParticleSpawnButton.Bounds = RectF{spawnButtonX, buttonY, buttonWidth, buttonHeight};
    ParticleSettingsButton.Bounds = RectF{settingsButtonX, settingsButtonY, buttonWidth, buttonHeight};

    constexpr float panelHeightEstimate = 600.0f;
    const float panelY = (std::max)(16.0f, settingsButtonY - panelHeightEstimate - 16.0f);
    ParticleSettingsPanel.SetBounds(RectF{panelX, panelY, panelWidth, 580.0f});

    if (context.Graphics.Render != nullptr)
    {
        CurrentParticleEmitterDesc = context.Graphics.Render->GetGpuParticleSystem().GetEmitterDesc();
    }
    ParticleSpawnButton.Label = CurrentParticleEmitterDesc.Enabled
        ? "SPAWN PARTICLES: ON"
        : "SPAWN PARTICLES: OFF";
    ParticleSettingsButton.Label = "SETTINGS";

    const POINT cursorPoint = GetMouseClientPosition(window->GetHandle());
    const bool isLeftMouseDown = RawInputHandler::Instance().IsLeftMouseDown();

    MouseState mouse{};
    mouse.X = static_cast<float>(cursorPoint.x);
    mouse.Y = static_cast<float>(cursorPoint.y);
    mouse.LeftDown = isLeftMouseDown;
    mouse.LeftPressed = isLeftMouseDown && !PreviousParticleUiLeftMouseDown;
    mouse.LeftReleased = !isLeftMouseDown && PreviousParticleUiLeftMouseDown;

    const bool clickedParticleButton = ParticleSpawnButton.HandleMouseClick(
        mouse.X,
        mouse.Y,
        mouse.LeftPressed
    );
    const bool clickedSettingsButton = ParticleSettingsButton.HandleMouseClick(
        mouse.X,
        mouse.Y,
        mouse.LeftPressed
    );

    if (clickedParticleButton)
    {
        if (context.Graphics.Render != nullptr)
        {
            GpuParticleSystem &particleSystem = context.Graphics.Render->GetGpuParticleSystem();
            GpuParticleEmitterDesc desc = particleSystem.GetEmitterDesc();
            desc.Enabled = !desc.Enabled;
            desc.DepthCollisionEnabled = false;
            ParticleSettingsPanel.ApplyToEmitterDesc(desc, GameConfig.PlayfieldHalfExtent);
            particleSystem.SetEmitterDesc(desc);
            CurrentParticleEmitterDesc = desc;
        }
    }
    else if (clickedSettingsButton)
    {
        ParticleSettingsPanel.Toggle();
    }
    else if (ParticleSettingsPanel.Update(mouse) == KatamariParticleUiAction::SliderMoved)
    {
        ApplyParticleSettingsToEmitter(context);
    }

    PreviousParticleUiLeftMouseDown = isLeftMouseDown;
}

void KatamariGame::RenderParticleUi(AppContext &context)
{
    if (context.Ui.Font == nullptr || context.Platform.MainWindow == nullptr)
    {
        return;
    }

    POINT cursorPoint = GetMouseClientPosition(context.Platform.MainWindow->GetHandle());
    const bool isSpawnHovered = ParticleSpawnButton.IsHovered(
        static_cast<float>(cursorPoint.x),
        static_cast<float>(cursorPoint.y)
    );
    const bool isSettingsHovered = ParticleSettingsButton.IsHovered(
        static_cast<float>(cursorPoint.x),
        static_cast<float>(cursorPoint.y)
    );

    ParticleSettingsPanel.Render(context.GetShapeRenderer2D());

    ButtonStyle buttonStyle{};
    buttonStyle.TextScale = 0.72f;
    ParticleSpawnButton.Draw(
        context.GetShapeRenderer2D(),
        *context.Ui.Font,
        buttonStyle,
        isSpawnHovered
    );
    ParticleSettingsButton.Draw(
        context.GetShapeRenderer2D(),
        *context.Ui.Font,
        buttonStyle,
        isSettingsHovered || ParticleSettingsPanel.IsOpen()
    );
}

void KatamariGame::ApplyParticleSettingsToEmitter(AppContext &context)
{
    if (context.Graphics.Render == nullptr)
    {
        return;
    }

    GpuParticleSystem &particleSystem = context.Graphics.Render->GetGpuParticleSystem();
    GpuParticleEmitterDesc desc = particleSystem.GetEmitterDesc();
    ParticleSettingsPanel.ApplyToEmitterDesc(desc, GameConfig.PlayfieldHalfExtent);
    particleSystem.SetEmitterDesc(desc);
    CurrentParticleEmitterDesc = desc;
}

void KatamariGame::DestroyAllPickupEntities()
{
    for (const auto &key: WorldContext.Pickups | std::views::keys)
    {
        SceneInstance.DestroyEntity(key);
    }

    WorldContext.Pickups.clear();
    WorldContext.TotalPickups = 0;
    WorldContext.CollectedCount = 0;
}

void KatamariGame::ResetLevel(AppContext &context)
{
    DestroyAllPickupEntities();
    SceneInstance.DestroyEntity(WorldContext.BallEntityId);
    WorldContext.BallEntityId = 0u;

    SceneInstance.ClearSystems();
    RegisterSceneSystems(context);

    KatamariLevelSetup::SpawnPlayerBallAndPickups(
        SceneInstance,
        context,
        WorldContext,
        GameConfig,
        PickupArchetypes,
        PlayerBallMeshModel
    );
    WorldContext.FollowCameraForMovement = &FollowCameraInstance;
    WorldContext.Config = &GameConfig;
}

void KatamariGame::UpdateDebugInput(AppContext &context)
{
    Keyboard const &keyboard = context.Input.System->GetKeyboard();
    if (keyboard.WasVirtualKeyPressed('R'))
    {
        ResetLevel(context);
    }

    if (keyboard.WasVirtualKeyPressed(VK_F3))
    {
        WorldContext.DebugDrawCollision = !WorldContext.DebugDrawCollision;
    }

    if (keyboard.WasVirtualKeyPressed(VK_F4))
    {
        SceneInstance.SetShadowCascadeDebugVisualizationEnabled(
            !SceneInstance.GetShadowCascadeDebugVisualizationEnabled()
        );
    }

    if (keyboard.WasVirtualKeyPressed(VK_F5))
    {
        if (context.Graphics.Render != nullptr)
        {
            context.Graphics.Render->SetGBufferDebugVisualizationEnabled(
                !context.Graphics.Render->IsGBufferDebugVisualizationEnabled()
            );
        }
    }
}

void KatamariGame::Update(AppContext &context, const float deltaTime)
{
    if (!Initialized)
    {
        return;
    }

    LastDeltaTime = deltaTime;
    FpsAccumulator += deltaTime;
    ++FpsFrames;
    if (FpsAccumulator >= 0.25f)
    {
        DisplayFps = static_cast<int>(std::lround(static_cast<float>(FpsFrames) / FpsAccumulator));
        FpsAccumulator = 0.0f;
        FpsFrames = 0;
    }

    UpdateDebugInput(context);

    SceneInstance.Update(context, deltaTime);

    TransformComponent const *const ballTransform = SceneInstance.TryGetTransformComponent(WorldContext.BallEntityId);
    VelocityComponent const *const ballVelocity = SceneInstance.TryGetVelocityComponent(WorldContext.BallEntityId);
    if (ballTransform != nullptr && ballVelocity != nullptr)
    {
        CameraController.Update(
            deltaTime,
            ballTransform->Local.Position,
            ballVelocity->LinearVelocity,
            WorldContext.BallVisualRadius,
            context.Input.System->IsRightMouseDown(),
            static_cast<float>(context.Input.System->GetMouseDeltaX()),
            static_cast<float>(context.Input.System->GetMouseDeltaY())
        );
    }

    if (WorldContext.DebugDrawCollision)
    {
        CollisionDebugDraw::DrawBoxColliderWorldBounds(
            SceneInstance,
            context.GetDebugDraw(),
            DirectX::SimpleMath::Color(0.2f, 0.85f, 0.35f, 1.0f)
        );
        CollisionDebugDraw::DrawContactNormalsFromLastFrame(
            SceneInstance,
            context.GetDebugDraw(),
            DirectX::SimpleMath::Color(1.0f, 0.4f, 0.1f, 1.0f),
            0.65f
        );
        CollisionDebugDraw::DrawSphereColliderWorldBounds(
            SceneInstance,
            context.GetDebugDraw(),
            DirectX::SimpleMath::Color(0.25f, 0.65f, 1.0f, 1.0f)
        );
        CollisionDebugDraw::DrawCollisionWorldBounds(
            SceneInstance,
            context.GetDebugDraw(),
            DirectX::SimpleMath::Color(0.8f, 0.8f, 0.2f, 1.0f)
        );
    }

    if (context.Graphics.Render != nullptr)
    {
        context.Graphics.Render->SetFrameGameplaySceneAndCamera(&SceneInstance, &FollowCameraInstance);
    }
}

void KatamariGame::Render(AppContext &context)
{
    if (!Initialized)
    {
        return;
    }

    const Window *const window = context.Platform.MainWindow;
    if (window == nullptr)
    {
        return;
    }

    const int width = window->GetWidth();
    const int height = window->GetHeight();
    if (width <= 0 || height <= 0)
    {
        return;
    }

    const RenderPassKind activeRenderPass = context.Graphics.Render != nullptr
        ? context.Graphics.Render->GetActiveRenderPassKind()
        : RenderPassKind::None;

    if (activeRenderPass == RenderPassKind::UserInterface)
    {
        const float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
        context.GetDebugDraw().Flush(
            context.GetPrimitiveRenderer3D(),
            FollowCameraInstance.GetViewMatrix(),
            FollowCameraInstance.GetProjectionMatrix(aspectRatio)
        );

        UpdateParticleUi(context);
        KatamariHud::Draw(
            context,
            WorldContext,
            DisplayFps,
            LastDeltaTime,
            context.Graphics.Render != nullptr && context.Graphics.Render->IsGBufferDebugVisualizationEnabled(),
            SceneInstance.GetShadowCascadeDebugVisualizationEnabled()
        );
        RenderParticleUi(context);
        return;
    }

    if (context.GetRenderMode() == RenderMode::Deferred
        && activeRenderPass != RenderPassKind::DeferredGeometry)
    {
        return;
    }

    const float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    const Matrix viewMatrix = FollowCameraInstance.GetViewMatrix();
    const Matrix projectionMatrix = FollowCameraInstance.GetProjectionMatrix(aspectRatio);

    SceneInstance.Render(context);

    PrimitiveRenderer3D &primitives = context.GetPrimitiveRenderer3D();
    ModelRenderer &modelRenderer = context.GetModelRenderer();
    const Vector3 cameraWorldPosition = CameraWorldPositionFromViewMatrix(viewMatrix);
    const float playfieldSpan = GameConfig.PlayfieldHalfExtent * 2.0f + GameConfig.WallThickness * 2.0f;
    const Vector3 skyCenter = Vector3(0.0f, playfieldSpan * 0.9f, 0.0f);
    for (const Vector3 &starDirection : NightStarPositions)
    {
        Vector3 starDirectionNormalized = starDirection;
        starDirectionNormalized.Normalize();
        const Vector3 starPosition = skyCenter + starDirectionNormalized * (playfieldSpan * 3.1f);
        const Matrix starWorld = Matrix::CreateScale(0.45f, 0.45f, 0.45f) * Matrix::CreateTranslation(starPosition);
        primitives.DrawSphere(starWorld, viewMatrix, projectionMatrix, DirectX::SimpleMath::Color(0.62f, 0.72f, 0.95f, 1.0f));
    }
    for (const PointLight3D &pointLight : KatamariLighting.PointLights)
    {
        const Matrix pointLightMarkerWorld =
            Matrix::CreateScale(0.65f, 0.65f, 0.65f) *
            Matrix::CreateTranslation(pointLight.Position);
        primitives.DrawSphere(pointLightMarkerWorld, viewMatrix, projectionMatrix, pointLight.LightColor);
    }

    if (MoonModel != nullptr && MoonModel->IsLoaded())
    {
        const Vector3 moonPosition = skyCenter + Vector3(0.44f, 0.38f, 0.76f) * (playfieldSpan * 2.7f);
        const Matrix moonWorld =
            Matrix::CreateScale(playfieldSpan * 0.52f, playfieldSpan * 0.52f, playfieldSpan * 0.52f) *
            Matrix::CreateTranslation(moonPosition);
        RenderMaterialParameters moonMaterial{};
        moonMaterial.BaseColor = DirectX::SimpleMath::Color(0.96f, 0.96f, 0.98f, 1.0f);
        moonMaterial.AmbientFactor = 0.12f;
        moonMaterial.SpecularPower = 64.0f;
        moonMaterial.SpecularColor = DirectX::SimpleMath::Color(0.75f, 0.75f, 0.8f, 1.0f);
        modelRenderer.DrawModelLit(
            *MoonModel,
            moonWorld,
            viewMatrix,
            projectionMatrix,
            cameraWorldPosition,
            KatamariLighting,
            moonMaterial
        );

        if (context.GetRenderMode() == RenderMode::Forward)
        {
            const Matrix moonGlowWorld = Matrix::CreateScale(playfieldSpan * 0.66f, playfieldSpan * 0.66f, playfieldSpan * 0.66f) *
                Matrix::CreateTranslation(moonPosition);
            primitives.DrawSphere(moonGlowWorld, viewMatrix, projectionMatrix, DirectX::SimpleMath::Color(0.48f, 0.56f, 0.85f, 0.12f));
        }
    }

    if (GroundModel != nullptr && GroundModel->IsLoaded())
    {
        if (!SceneInstance.IsEntityAlive(GroundEntityId))
        {
            const Matrix groundWorld =
                Matrix::CreateScale(playfieldSpan, 1.0f, playfieldSpan) *
                Matrix::CreateTranslation(0.0f, GroundModelVerticalOffset, 0.0f);
            RenderMaterialParameters groundMaterial{};
            groundMaterial.BaseColor = DirectX::SimpleMath::Color(0.62f, 0.7f, 0.88f, 1.0f);
            groundMaterial.AmbientFactor = 0.09f;
            groundMaterial.SpecularPower = 92.0f;
            groundMaterial.SpecularColor = DirectX::SimpleMath::Color(0.72f, 0.72f, 0.72f, 1.0f);
            modelRenderer.DrawModelLit(
                *GroundModel,
                groundWorld,
                viewMatrix,
                projectionMatrix,
                cameraWorldPosition,
                KatamariLighting,
                groundMaterial
            );
        }
    }
    else
    {
        const Matrix floorWorld =
            Matrix::CreateScale(playfieldSpan, GameConfig.FloorColliderHalfThickness * 2.0f, playfieldSpan) *
            Matrix::CreateTranslation(0.0f, -GameConfig.FloorColliderHalfThickness, 0.0f);
        RenderMaterialParameters floorMaterial{};
        floorMaterial.BaseColor = DirectX::SimpleMath::Color(0.22f, 0.48f, 0.28f, 1.0f);
        floorMaterial.AmbientFactor = 0.18f;
        floorMaterial.SpecularPower = 64.0f;
        floorMaterial.SpecularColor = DirectX::SimpleMath::Color(0.55f, 0.55f, 0.55f, 1.0f);
        primitives.DrawBoxLit(
            floorWorld,
            viewMatrix,
            projectionMatrix,
            cameraWorldPosition,
            KatamariLighting,
            floorMaterial
        );
    }

}

void KatamariGame::Shutdown(AppContext &context)
{
    if (context.Graphics.Render != nullptr)
    {
        GpuParticleEmitterDesc particles = context.Graphics.Render->GetGpuParticleSystem().GetEmitterDesc();
        particles.Enabled = false;
        context.Graphics.Render->GetGpuParticleSystem().SetEmitterDesc(particles);
    }

    Initialized = false;
}
