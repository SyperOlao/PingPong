#include "Game/LightingTest/LightingTestGame.h"

#include "Core/App/AppContext.h"
#include "Core/Assets/AssetCache.h"
#include "Core/Graphics/ModelRenderer.h"
#include "Core/Graphics/Rendering/Lighting/SceneLighting3D.h"
#include "Core/Graphics/Rendering/Renderables/RenderMaterialParameters.h"
#include "Core/Graphics/Rendering/PrimitiveRenderer3D.h"
#include "Core/Input/InputSystem.h"
#include "Core/Input/Keyboard.h"
#include "Core/Input/RawInputHandler.h"
#include "Core/Platform/Window.h"

#include <cmath>
#include <stdexcept>

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

void LightingTestGame::Initialize(AppContext &context)
{
    if (!context.IsValid())
    {
        throw std::runtime_error("LightingTestGame::Initialize received invalid AppContext.");
    }

    SceneCamera.SetPosition(Vector3(0.0f, 3.0f, -10.0f));
    SceneCamera.SetRotation(0.0f, 0.1f);

    if (AssetCache *const assetCache = context.Assets.Cache; assetCache != nullptr)
    {
        CubeModel = assetCache->LoadModel("Core/Data/Cube.fbx");
        SphereModel = assetCache->LoadModel("Core/Data/Sphere.fbx");
    }

    Lighting = SceneLighting3DCreateDefaultOutdoor();
    if (!Lighting.DirectionalLights.empty())
    {
        Lighting.DirectionalLights[0].Direction = Vector3(-0.32f, -1.0f, -0.26f);
        Lighting.DirectionalLights[0].Intensity = 1.25f;
    }

    PointLight3D pointLight{};
    pointLight.Position = Vector3(0.0f, 3.0f, 0.0f);
    pointLight.LightColor = DirectX::SimpleMath::Color(1.0f, 0.92f, 0.84f, 1.0f);
    pointLight.Intensity = 3.2f;
    pointLight.Range = 10.0f;
    Lighting.PointLights.push_back(pointLight);

    IsInitialized = true;
}

void LightingTestGame::Update(AppContext &context, const float deltaTime)
{
    if (!IsInitialized)
    {
        return;
    }

    AccumulatedTimeSeconds += deltaTime;
    UpdateCamera(context, deltaTime);
    RawInputHandler::Instance().ClearFrameDeltas();
}

void LightingTestGame::Render(AppContext &context)
{
    if (!IsInitialized)
    {
        return;
    }

    const Window *const mainWindow = context.Platform.MainWindow;
    if (mainWindow == nullptr)
    {
        return;
    }

    const int width = mainWindow->GetWidth();
    const int height = mainWindow->GetHeight();
    if (width <= 0 || height <= 0)
    {
        return;
    }

    const float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    const Matrix viewMatrix = SceneCamera.GetViewMatrix();
    const Matrix projectionMatrix = SceneCamera.GetProjectionMatrix(aspectRatio);
    const Vector3 cameraWorldPosition = SceneCamera.GetPosition();

    PrimitiveRenderer3D &primitiveRenderer = context.GetPrimitiveRenderer3D();
    ModelRenderer &modelRenderer = context.GetModelRenderer();

    RenderMaterialParameters floorMaterial{};
    floorMaterial.BaseColor = DirectX::SimpleMath::Color(0.22f, 0.24f, 0.28f, 1.0f);
    floorMaterial.AmbientFactor = 0.12f;
    floorMaterial.SpecularPower = 16.0f;
    floorMaterial.SpecularColor = DirectX::SimpleMath::Color(0.14f, 0.14f, 0.14f, 1.0f);
    const Matrix floorWorld =
        Matrix::CreateScale(18.0f, 0.4f, 18.0f) *
        Matrix::CreateTranslation(0.0f, -0.2f, 0.0f);
    primitiveRenderer.DrawBoxLit(
        floorWorld,
        viewMatrix,
        projectionMatrix,
        cameraWorldPosition,
        Lighting,
        floorMaterial
    );

    if (CubeModel != nullptr && CubeModel->IsLoaded())
    {
        RenderMaterialParameters cubeMaterial{};
        cubeMaterial.BaseColor = DirectX::SimpleMath::Color(0.32f, 0.72f, 0.96f, 1.0f);
        cubeMaterial.SpecularPower = 52.0f;
        cubeMaterial.SpecularColor = DirectX::SimpleMath::Color(0.85f, 0.9f, 1.0f, 1.0f);
        const Matrix cubeWorld =
            Matrix::CreateScale(0.01f, 0.01f, 0.01f) *
            Matrix::CreateRotationY(AccumulatedTimeSeconds * 0.45f) *
            Matrix::CreateTranslation(-2.2f, 0.8f, 0.6f);
        modelRenderer.DrawModelLit(
            *CubeModel,
            cubeWorld,
            viewMatrix,
            projectionMatrix,
            cameraWorldPosition,
            Lighting,
            cubeMaterial
        );
    }

    if (SphereModel != nullptr && SphereModel->IsLoaded())
    {
        RenderMaterialParameters sphereMaterial{};
        sphereMaterial.BaseColor = DirectX::SimpleMath::Color(0.95f, 0.42f, 0.26f, 1.0f);
        sphereMaterial.SpecularPower = 96.0f;
        sphereMaterial.SpecularColor = DirectX::SimpleMath::Color(1.0f, 0.86f, 0.78f, 1.0f);
        const float bobOffset = std::sin(AccumulatedTimeSeconds * 1.5f) * 0.2f;
        const Matrix sphereWorld =
            Matrix::CreateScale(0.01f, 0.01f, 0.01f) *
            Matrix::CreateTranslation(2.0f, 1.35f + bobOffset, -0.4f);
        modelRenderer.DrawModelLit(
            *SphereModel,
            sphereWorld,
            viewMatrix,
            projectionMatrix,
            cameraWorldPosition,
            Lighting,
            sphereMaterial
        );
    }
}

void LightingTestGame::UpdateCamera(const AppContext &context, const float deltaTime)
{
    const Keyboard &keyboard = context.Input.System->GetKeyboard();
    const float moveSpeed = 9.0f;
    const float rotateSpeed = 1.7f;
    const float mouseSensitivity = 0.0035f;

    if (keyboard.IsKeyDown(Key::W))
    {
        SceneCamera.MoveForward(moveSpeed * deltaTime);
    }
    if (keyboard.IsKeyDown(Key::S))
    {
        SceneCamera.MoveForward(-moveSpeed * deltaTime);
    }
    if (keyboard.IsKeyDown(Key::D))
    {
        SceneCamera.MoveRight(moveSpeed * deltaTime);
    }
    if (keyboard.IsKeyDown(Key::A))
    {
        SceneCamera.MoveRight(-moveSpeed * deltaTime);
    }

    float yawDelta = 0.0f;
    float pitchDelta = 0.0f;
    if (context.Input.System->IsRightMouseDown())
    {
        yawDelta += static_cast<float>(context.Input.System->GetMouseDeltaX()) * mouseSensitivity;
        pitchDelta -= static_cast<float>(context.Input.System->GetMouseDeltaY()) * mouseSensitivity;
    }

    if (keyboard.IsKeyDown(Key::Left))
    {
        yawDelta -= rotateSpeed * deltaTime;
    }
    if (keyboard.IsKeyDown(Key::Right))
    {
        yawDelta += rotateSpeed * deltaTime;
    }
    if (keyboard.IsKeyDown(Key::Up))
    {
        pitchDelta += rotateSpeed * deltaTime;
    }
    if (keyboard.IsKeyDown(Key::Down))
    {
        pitchDelta -= rotateSpeed * deltaTime;
    }

    SceneCamera.AddRotation(yawDelta, pitchDelta);
}
