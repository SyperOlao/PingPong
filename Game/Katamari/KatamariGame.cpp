#include "Game/Katamari/KatamariGame.h"

#include "Core/App/AppContext.h"
#include "Core/Gameplay/CollisionDebugDraw.h"
#include "Core/Gameplay/CollisionSystem.h"
#include "Core/Gameplay/RenderSystem.h"
#include "Core/Gameplay/TransformSystem.h"
#include "Core/Gameplay/VelocityIntegrationSystem.h"
#include "Core/Graphics/Rendering/RenderContext.h"
#include "Core/Input/InputSystem.h"
#include "Core/Input/Keyboard.h"
#include "Game/Katamari/Data/KatamariPickupCatalog.h"
#include "Game/Katamari/KatamariLevelSetup.h"
#include "Game/Katamari/Systems/KatamariAbsorptionSystem.h"
#include "Game/Katamari/Systems/KatamariBallControllerSystem.h"
#include "Game/Katamari/Systems/KatamariSphereWorldResolveSystem.h"
#include "Game/Katamari/UI/KatamariHud.h"
#include "Core/Platform/Window.h"
#include "Core/Graphics/Rendering/PrimitiveRenderer3D.h"

#include <cmath>
#include <memory>
#include <stdexcept>

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

void KatamariGame::Initialize(AppContext &context)
{
    if (!context.IsValid())
    {
        throw std::runtime_error("KatamariGame::Initialize received invalid AppContext.");
    }

    if (context.Graphics.Render != nullptr && !ForwardRenderPipelineBuilt)
    {
        context.Graphics.Render->BuildDefaultForwardRenderPipeline();
        ForwardRenderPipelineBuilt = true;
    }

    PickupArchetypes = KatamariPickupCatalog::BuildDefaultPickupArchetypes();

    SceneInstance.SetForwardLightingEnabled(false);
    SceneInstance.SetDirectionalShadowMappingEnabled(false);
    SceneInstance.SetActiveCamera(&FollowCameraInstance);

    FollowCameraInstance.SetOffsetFromTarget(Vector3(0.0f, 9.0f, -26.0f));
    FollowCameraInstance.SetPositionLagSeconds(0.14f);
    FollowCameraInstance.SetOrientationLagSeconds(0.2f);
    FollowCameraInstance.SetZoomFromRadiusParameters(GameConfig.InitialBallRadius, 0.7f, 2.2f);
    CameraController.Initialize(FollowCameraInstance);

    RegisterSceneSystems(context);

    if (!StaticWorldCreated)
    {
        KatamariLevelSetup::CreateStaticWorldCollision(SceneInstance, GameConfig);
        StaticWorldCreated = true;
    }

    KatamariLevelSetup::SpawnPlayerBallAndPickups(
        SceneInstance,
        context,
        WorldContext,
        GameConfig,
        PickupArchetypes
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
    SceneInstance.AddSystem(std::make_unique<TransformSystem>());
    SceneInstance.AddSystem(std::make_unique<CollisionSystem>());
    SceneInstance.AddSystem(std::make_unique<KatamariSphereWorldResolveSystem>(&WorldContext));
    SceneInstance.AddSystem(std::make_unique<KatamariAbsorptionSystem>(&WorldContext));
    SceneInstance.AddSystem(std::make_unique<RenderSystem>());
    SceneInstance.InitializeSystems(context);
}

void KatamariGame::DestroyAllPickupEntities()
{
    for (auto const &entry : WorldContext.Pickups)
    {
        SceneInstance.DestroyEntity(entry.first);
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
        PickupArchetypes
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
            WorldContext.BallRadius
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
    }
}

void KatamariGame::Render(AppContext &context)
{
    if (!Initialized)
    {
        return;
    }

    Window *const window = context.Platform.MainWindow;
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

    const float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    const Matrix viewMatrix = FollowCameraInstance.GetViewMatrix();
    const Matrix projectionMatrix = FollowCameraInstance.GetProjectionMatrix(aspectRatio);

    SceneInstance.Render(context);

    PrimitiveRenderer3D &primitives = context.GetPrimitiveRenderer3D();
    const float playfieldSpan = GameConfig.PlayfieldHalfExtent * 2.0f + GameConfig.WallThickness * 2.0f;
    const Matrix floorWorld =
        Matrix::CreateScale(playfieldSpan, GameConfig.FloorColliderHalfThickness * 2.0f, playfieldSpan) *
        Matrix::CreateTranslation(0.0f, -GameConfig.FloorColliderHalfThickness, 0.0f);
    primitives.DrawBox(
        floorWorld,
        viewMatrix,
        projectionMatrix,
        DirectX::SimpleMath::Color(0.22f, 0.48f, 0.28f, 1.0f)
    );

    TransformComponent const *const ballTransform = SceneInstance.TryGetTransformComponent(WorldContext.BallEntityId);
    if (ballTransform != nullptr)
    {
        const float diameter = WorldContext.BallRadius * 2.0f;
        const Matrix ballWorld =
            Matrix::CreateScale(diameter, diameter, diameter) *
            Matrix::CreateTranslation(ballTransform->Local.Position);
        primitives.DrawSphere(
            ballWorld,
            viewMatrix,
            projectionMatrix,
            DirectX::SimpleMath::Color(0.95f, 0.35f, 0.2f, 1.0f)
        );
    }

    KatamariHud::Draw(
        context,
        WorldContext,
        DisplayFps,
        LastDeltaTime
    );
}

void KatamariGame::Shutdown(AppContext &)
{
    Initialized = false;
}
