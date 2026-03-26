#include "Game/Katamari/KatamariGame.h"

#include "Core/App/AppContext.h"
#include "Core/Assets/AssetCache.h"
#include "Core/Gameplay/CollisionDebugDraw.h"
#include "Core/Gameplay/CollisionSystem.h"
#include "Core/Gameplay/RenderSystem.h"
#include "Core/Gameplay/TransformSystem.h"
#include "Core/Gameplay/VelocityIntegrationSystem.h"
#include "Core/Graphics/ModelRenderer.h"
#include "Core/Graphics/Rendering/Lighting/SceneLighting3D.h"
#include "Core/Graphics/Rendering/RenderContext.h"
#include "Core/Graphics/Rendering/Renderables/RenderMaterialParameters.h"
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

#include <DirectXCollision.h>
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

    if (context.Graphics.Render != nullptr && !ForwardRenderPipelineBuilt)
    {
        context.Graphics.Render->BuildDefaultForwardRenderPipeline();
        ForwardRenderPipelineBuilt = true;
    }

    PickupArchetypes = KatamariPickupCatalog::BuildDefaultPickupArchetypes();

    SceneInstance.SetForwardLightingEnabled(false);
    SceneInstance.SetDirectionalShadowMappingEnabled(false);
    SceneInstance.SetActiveCamera(&FollowCameraInstance);

    KatamariLighting = SceneLighting3DCreateDefaultOutdoor();
    if (!KatamariLighting.DirectionalLights.empty())
    {
        KatamariLighting.DirectionalLights[0].Direction = Vector3(-0.18f, -1.0f, -0.32f);
        KatamariLighting.DirectionalLights[0].Intensity = 0.28f;
        KatamariLighting.DirectionalLights[0].LightColor = DirectX::SimpleMath::Color(0.52f, 0.63f, 0.92f, 1.0f);
    }
    SceneInstance.GetSceneLightingDescriptor() = KatamariLighting;

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
    }

    GroundModelVerticalOffset = 0.0f;
    if (GroundModel != nullptr && GroundModel->IsLoaded() && GroundModel->Get() != nullptr)
    {
        const DirectX::BoundingBox groundBounds = ComputeMergedModelBoundingBox(GroundModel);
        const float groundTopLocalY = groundBounds.Center.y + groundBounds.Extents.y;
        GroundModelVerticalOffset = -groundTopLocalY;
    }

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
            WorldContext.BallRadius,
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

    if (MoonModel != nullptr && MoonModel->IsLoaded())
    {
        const Vector3 moonPosition = skyCenter + Vector3(0.44f, 0.38f, 0.76f) * (playfieldSpan * 2.7f);
        const Matrix moonWorld =
            Matrix::CreateScale(playfieldSpan * 0.52f, playfieldSpan * 0.52f, playfieldSpan * 0.52f) *
            Matrix::CreateTranslation(moonPosition);
        RenderMaterialParameters moonMaterial{};
        moonMaterial.BaseColor = DirectX::SimpleMath::Color(1.0f, 1.0f, 1.0f, 1.0f);
        modelRenderer.DrawModel(*MoonModel, moonWorld, viewMatrix, projectionMatrix, moonMaterial);

        const Matrix moonGlowWorld = Matrix::CreateScale(playfieldSpan * 0.66f, playfieldSpan * 0.66f, playfieldSpan * 0.66f) *
            Matrix::CreateTranslation(moonPosition);
        primitives.DrawSphere(moonGlowWorld, viewMatrix, projectionMatrix, DirectX::SimpleMath::Color(0.48f, 0.56f, 0.85f, 0.12f));
    }

    if (GroundModel != nullptr && GroundModel->IsLoaded())
    {
        const Matrix groundWorld =
            Matrix::CreateScale(playfieldSpan, 1.0f, playfieldSpan) *
            Matrix::CreateTranslation(0.0f, GroundModelVerticalOffset, 0.0f);
        RenderMaterialParameters groundMaterial{};
        groundMaterial.BaseColor = DirectX::SimpleMath::Color(0.62f, 0.7f, 0.88f, 1.0f);
        modelRenderer.DrawModel(*GroundModel, groundWorld, viewMatrix, projectionMatrix, groundMaterial);
    }
    else
    {
        const Matrix floorWorld =
            Matrix::CreateScale(playfieldSpan, GameConfig.FloorColliderHalfThickness * 2.0f, playfieldSpan) *
            Matrix::CreateTranslation(0.0f, -GameConfig.FloorColliderHalfThickness, 0.0f);
        RenderMaterialParameters floorMaterial{};
        floorMaterial.BaseColor = DirectX::SimpleMath::Color(0.22f, 0.48f, 0.28f, 1.0f);
        floorMaterial.AmbientFactor = 0.22f;
        floorMaterial.SpecularPower = 12.0f;
        floorMaterial.SpecularColor = DirectX::SimpleMath::Color(0.12f, 0.12f, 0.12f, 1.0f);
        primitives.DrawBoxLit(
            floorWorld,
            viewMatrix,
            projectionMatrix,
            cameraWorldPosition,
            KatamariLighting,
            floorMaterial
        );
    }

    if (TransformComponent const *const ballTransform = SceneInstance.TryGetTransformComponent(WorldContext.BallEntityId); ballTransform != nullptr)
    {
        const Matrix contactShadowWorld =
            Matrix::CreateScale(WorldContext.BallRadius * 1.12f, 0.02f, WorldContext.BallRadius * 1.12f) *
            Matrix::CreateTranslation(ballTransform->Local.Position.x, 0.01f, ballTransform->Local.Position.z);
        primitives.DrawSphere(
            contactShadowWorld,
            viewMatrix,
            projectionMatrix,
            DirectX::SimpleMath::Color(0.02f, 0.03f, 0.03f, 0.45f)
        );

        const Matrix ballWorld =
            Matrix::CreateScale(WorldContext.BallRadius, WorldContext.BallRadius, WorldContext.BallRadius) *
            Matrix::CreateTranslation(ballTransform->Local.Position);
        RenderMaterialParameters ballMaterial{};
        ballMaterial.BaseColor = DirectX::SimpleMath::Color(0.95f, 0.35f, 0.2f, 1.0f);
        ballMaterial.AmbientFactor = 0.2f;
        ballMaterial.SpecularPower = 96.0f;
        ballMaterial.SpecularColor = DirectX::SimpleMath::Color(1.0f, 0.9f, 0.8f, 1.0f);
        primitives.DrawSphereLit(
            ballWorld,
            viewMatrix,
            projectionMatrix,
            cameraWorldPosition,
            KatamariLighting,
            ballMaterial
        );
    }

    if (WorldContext.DebugDrawCollision)
    {
        context.GetDebugDraw().Flush(primitives, viewMatrix, projectionMatrix);
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
