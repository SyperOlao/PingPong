cmake_minimum_required(VERSION 3.21)

project(PingPong LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(PingPong
        main.cpp

        Core/App/Application.cpp
        Core/App/Application.h
        Core/App/ApplicationDefaults.h
        Core/App/Timer.cpp
        Core/App/Timer.h
        Core/App/IGame.h
        Core/App/EngineServices.h
        Core/App/AppContext.h
        Core/App/AppContext.cpp
        Core/App/FatalErrorReport.h
        Core/App/FatalErrorReport.cpp

        Core/Platform/Window.cpp
        Core/Platform/Window.h

        Core/Input/InputSystem.cpp
        Core/Input/InputSystem.h
        Core/Input/Keyboard.cpp
        Core/Input/Keyboard.h
        Core/Input/RawInputHandler.cpp
        Core/Input/RawInputHandler.h

        Core/Graphics/GraphicsDevice.cpp
        Core/Graphics/GraphicsDevice.h
        Core/Graphics/D3d11Helpers.h
        Core/Graphics/ShaderCompiler.cpp
        Core/Graphics/ShaderCompiler.h
        Core/Graphics/Color.h
        Core/Graphics/OrbitCamera.cpp
        Core/Graphics/OrbitCamera.h
        Core/Graphics/FpsCamera.cpp
        Core/Graphics/FpsCamera.h
        Core/Graphics/MeshData.h
        Core/Graphics/MeshGenerator.h
        Core/Graphics/MeshGenerator.cpp
        Core/Graphics/MeshLitData.h
        Core/Graphics/Rendering/Lighting/LightTypes3D.h
        Core/Graphics/Rendering/Lighting/ShaderConstants3D.h
        Core/Graphics/Rendering/Lighting/SceneLighting3D.h
        Core/Graphics/Rendering/Lighting/SceneLighting3D.cpp
        Core/Graphics/Rendering/Lighting/ForwardPhongMaterialGpu.h
        Core/Graphics/Rendering/ForwardPhongGpuUpload.h
        Core/Graphics/Rendering/ForwardPhongGpuUpload.cpp
        Core/Graphics/ProjectionMode.h
        Core/Graphics/Camera.h
        Core/Graphics/Camera.cpp
        Core/Graphics/FollowCamera.h
        Core/Graphics/FollowCamera.cpp
        Core/Graphics/Vertex3D.h
        Core/Graphics/ObjectConstants.h
        Core/Graphics/ModelAsset.h
        Core/Graphics/ModelAsset.cpp
        Core/Graphics/ModelAssetAssimp.cpp
        Core/Graphics/ModelRenderer.h
        Core/Graphics/ModelRenderer.cpp
        Core/Graphics/Debug/DebugDrawQueue.h
        Core/Graphics/Debug/DebugDrawQueue.cpp
        Core/Graphics/Texture2DAsset.h
        Core/Graphics/Texture2DAsset.cpp
        Core/Graphics/Particles/GpuParticleTypes.h
        Core/Graphics/Particles/GpuParticleSystem.h
        Core/Graphics/Particles/GpuParticleSystem.cpp

        Core/Graphics/Rendering/ShapeRenderer2D.cpp
        Core/Graphics/Rendering/ShapeRenderer2D.h
        Core/Graphics/Rendering/PrimitiveRenderer3D.cpp
        Core/Graphics/Rendering/PrimitiveRenderer3D.h
        Core/Graphics/Rendering/RenderContext.cpp
        Core/Graphics/Rendering/RenderContext.h
        Core/Graphics/Rendering/FrameRenderTypes.h
        Core/Graphics/Rendering/FrameRenderer.h
        Core/Graphics/Rendering/FrameRenderer.cpp
        Core/Graphics/Rendering/Pipeline/FrameRenderResources.h
        Core/Graphics/Rendering/Pipeline/FrameRenderResources.cpp
        Core/Graphics/Rendering/Pipeline/FramePassRenderContext.h
        Core/Graphics/Rendering/Pipeline/FramePassRenderContext.cpp
        Core/Graphics/Rendering/Pipeline/IRenderPass.h
        Core/Graphics/Rendering/Pipeline/FrameRenderPipeline.h
        Core/Graphics/Rendering/Pipeline/FrameRenderPipeline.cpp
        Core/Graphics/Rendering/Pipeline/Passes/ShadowDepthRenderPass.h
        Core/Graphics/Rendering/Pipeline/Passes/ShadowDepthRenderPass.cpp
        Core/Graphics/Rendering/Pipeline/Passes/MainGeometryBindRenderPass.h
        Core/Graphics/Rendering/Pipeline/Passes/MainGeometryBindRenderPass.cpp
        Core/Graphics/Rendering/Pipeline/Passes/GameRenderPass.h
        Core/Graphics/Rendering/Pipeline/Passes/GameRenderPass.cpp
        Core/Graphics/Rendering/Pipeline/Passes/DeferredGeometryRenderPass.h
        Core/Graphics/Rendering/Pipeline/Passes/DeferredGeometryRenderPass.cpp
        Core/Graphics/Rendering/Pipeline/Passes/DeferredLightingRenderPass.h
        Core/Graphics/Rendering/Pipeline/Passes/DeferredLightingRenderPass.cpp
        Core/Graphics/Rendering/Pipeline/Passes/DeferredCompositeRenderPass.h
        Core/Graphics/Rendering/Pipeline/Passes/DeferredCompositeRenderPass.cpp
        Core/Graphics/Rendering/Pipeline/Passes/GBufferDebugRenderPass.h
        Core/Graphics/Rendering/Pipeline/Passes/GBufferDebugRenderPass.cpp
        Core/Graphics/Rendering/Pipeline/Passes/ParticlesRenderPass.h
        Core/Graphics/Rendering/Pipeline/Passes/ParticlesRenderPass.cpp
        Core/Graphics/Rendering/Pipeline/Passes/DebugOverlayRenderPass.h
        Core/Graphics/Rendering/Pipeline/Passes/DebugOverlayRenderPass.cpp
        Core/Graphics/Rendering/Pipeline/Passes/UserInterfaceRenderPass.h
        Core/Graphics/Rendering/Pipeline/Passes/UserInterfaceRenderPass.cpp
        Core/Graphics/Rendering/Deferred/GBufferLayout.h
        Core/Graphics/Rendering/Deferred/DeferredShaderConstants3D.h
        Core/Graphics/Rendering/Deferred/ColorRenderTarget2D.h
        Core/Graphics/Rendering/Deferred/ColorRenderTarget2D.cpp
        Core/Graphics/Rendering/Deferred/GBufferResources.h
        Core/Graphics/Rendering/Deferred/GBufferResources.cpp
        Core/Graphics/Rendering/Deferred/DeferredFrameResources.h
        Core/Graphics/Rendering/Deferred/DeferredFrameResources.cpp
        Core/Graphics/Rendering/Shadows/ShadowShaderConstants3D.h
        Core/Graphics/Rendering/Shadows/ShadowProjection.h
        Core/Graphics/Rendering/Shadows/ShadowProjection.cpp
        Core/Graphics/Rendering/Shadows/CascadedShadowMapMath.h
        Core/Graphics/Rendering/Shadows/CascadedShadowMapMath.cpp
        Core/Graphics/Rendering/Shadows/DepthRenderTarget2D.h
        Core/Graphics/Rendering/Shadows/DepthRenderTarget2D.cpp
        Core/Graphics/Rendering/Shadows/ShadowComparisonSamplerState.h
        Core/Graphics/Rendering/Shadows/ShadowComparisonSamplerState.cpp
        Core/Graphics/Rendering/Shadows/ShadowDepthPassRasterizerState.h
        Core/Graphics/Rendering/Shadows/ShadowDepthPassRasterizerState.cpp
        Core/Graphics/Rendering/Shadows/DirectionalShadowResources.h
        Core/Graphics/Rendering/Shadows/DirectionalShadowResources.cpp
        Core/Graphics/Rendering/Renderables/RenderMaterialParameters.h
        Core/Graphics/Rendering/Renderables/RenderTransform3D.h
        Core/Graphics/Rendering/Renderables/RenderGeometry3D.h
        Core/Graphics/Rendering/Renderables/Renderable3D.h
        Core/Graphics/Rendering/Renderables/SceneRenderData3D.h
        Core/Graphics/Rendering/Renderables/SceneRenderer3D.h
        Core/Graphics/Rendering/Renderables/SceneRenderer3D.cpp

        Core/Assets/AssetCache.h
        Core/Assets/AssetCache.cpp
        Core/Assets/AssetPathResolver.h
        Core/Assets/AssetPathResolver.cpp

        Core/Physics/AABB.h
        Core/Physics/BoundingSphere.h
        Core/Physics/Collision3D.h
        Core/Physics/CollisionFoundation.h
        Core/Physics/Collision2D/AABB2D.h
        Core/Physics/Collision2D/Circle2D.h
        Core/Physics/Collision2D/CollisionContact2D.h
        Core/Physics/Collision2D/CollisionQueries2D.cpp
        Core/Physics/Collision2D/CollisionQueries2D.h
        Core/Physics/Collision2D/HorizontalBoundsExit.h
        Core/Physics/Collision3D/AxisAlignedBox3D.h
        Core/Physics/UniformSpatialGrid3D.h
        Core/Physics/UniformSpatialGrid3D.cpp
        Core/Physics/Collision3D/BoundingSphere3D.h
        Core/Physics/Collision3D/CollisionContact3D.h
        Core/Physics/Collision3D/CollisionQueries3D.cpp
        Core/Physics/Collision3D/CollisionQueries3D.h
        Core/Physics/HorizontalBoundsExit.h
        Core/Physics/PlaneConstraint.h
        Core/Physics/PlaneConstraint.cpp

        Core/Math/SpatialMath.h
        Core/Math/Transform3D.h
        Core/Math/Transform3D.cpp
        Core/Math/Transform2D.h
        Core/Math/Transform2D.cpp

        Core/Gameplay/EntityId.h
        Core/Gameplay/GameplayComponents.h
        Core/Gameplay/SceneCollisionTypes.h
        Core/Gameplay/ISceneSystem.h
        Core/Gameplay/ISceneSystem.cpp
        Core/Gameplay/Entity.h
        Core/Gameplay/Entity.cpp
        Core/Gameplay/Scene.h
        Core/Gameplay/AttachmentTransformHelpers.cpp
        Core/Gameplay/AttachmentTransformHelpers.h
        Core/Gameplay/Scene.cpp
        Core/Gameplay/TransformSystem.h
        Core/Gameplay/TransformSystem.cpp
        Core/Gameplay/VelocityIntegrationSystem.h
        Core/Gameplay/VelocityIntegrationSystem.cpp
        Core/Gameplay/CollisionSystem.h
        Core/Gameplay/CollisionSystem.cpp
        Core/Gameplay/RenderSystem.h
        Core/Gameplay/RenderSystem.cpp
        Core/Gameplay/SphereStaticWorldCollisionResolver.h
        Core/Gameplay/SphereStaticWorldCollisionResolver.cpp
        Core/Gameplay/CollisionDebugDraw.h
        Core/Gameplay/CollisionDebugDraw.cpp

        Core/UI/BitmapFont.cpp
        Core/UI/BitmapFont.h
        Core/UI/Button.cpp
        Core/UI/Button.h
        Core/UI/Switcher.cpp
        Core/UI/Switcher.h
        Core/UI/Widgets/SliderWidget.cpp
        Core/UI/Widgets/SliderWidget.h
        Core/UI/Widgets/CollapsibleSection.cpp
        Core/UI/Widgets/CollapsibleSection.h
        Core/UI/Common/UITheme.h
        Core/UI/Common/RectF.h
        Core/UI/Common/MouseState.h

        Core/Audio/AudioSystem.cpp
        Core/Audio/AudioSystem.h

        Game/Pong/PongGame.cpp
        Game/Pong/PongGame.h
        Game/Pong/PongTypes.h
        Game/Pong/Common/Types.h
        Game/Pong/Entities/Paddle.cpp
        Game/Pong/Entities/Ball.cpp
        Game/Pong/Systems/PongRules.cpp
        Game/Pong/Systems/PaddleAI.cpp
        Game/Pong/Systems/PongCollisionSystem.cpp
        Game/Pong/Systems/PongCollisionSystem.h
        Game/Pong/PongScene.cpp
        Game/Pong/Entities/ScoreBoard.cpp
        Game/Pong/Entities/Wall.cpp
        Game/Pong/Entities/PlayField.cpp
        Game/Pong/Entities/CenterLine.cpp
        Game/Pong/UI/PongUI.cpp
        Game/Pong/UI/PongUI.h

        Game/SolarSystem/SolarSystemGame.cpp
        Game/SolarSystem/SolarSystemGame.h
        Game/SolarSystem/SolarSystemScene.cpp
        Game/SolarSystem/SolarSystemScene.h
        Game/SolarSystem/Entities/PlanetBody.cpp
        Game/SolarSystem/Entities/PlanetBody.h
        Game/SolarSystem/Systems/OrbitMath.h
        Game/SolarSystem/Systems/OrbitMath.cpp
        Game/SolarSystem/Entities/OrbitalBody.cpp
        Game/SolarSystem/Entities/OrbitalBody.h
        Game/SolarSystem/UI/SolarSystemSettingsPanel.cpp
        Game/SolarSystem/UI/SolarSystemSettingsPanel.h
        Game/SolarSystem/Systems/SolarSystemTuning.h
        Game/SolarSystem/Rendering/SpaceBackdropRenderer.cpp
        Game/SolarSystem/Rendering/SpaceBackdropRenderer.h
        Game/SolarSystem/Systems/SolarSystemEngineAudioController.cpp
        Game/SolarSystem/Systems/SolarSystemEngineAudioController.h

        Game/Katamari/KatamariGame.cpp
        Game/Katamari/KatamariGame.h
        Game/Katamari/KatamariSceneLighting.cpp
        Game/Katamari/KatamariSceneLighting.h
        Game/Katamari/KatamariLevelSetup.cpp
        Game/Katamari/KatamariLevelSetup.h
        Game/Katamari/KatamariTypes.h
        Game/Katamari/Systems/KatamariSpawner.cpp
        Game/Katamari/Systems/KatamariSpawner.h
        Game/Katamari/Systems/KatamariAbsorptionSystem.cpp
        Game/Katamari/Systems/KatamariAbsorptionSystem.h
        Game/Katamari/Systems/KatamariBallControllerSystem.cpp
        Game/Katamari/Systems/KatamariBallControllerSystem.h
        Game/Katamari/Systems/KatamariBallRollSystem.cpp
        Game/Katamari/Systems/KatamariBallRollSystem.h
        Game/Katamari/Systems/KatamariBallVisualRadiusSystem.cpp
        Game/Katamari/Systems/KatamariBallVisualRadiusSystem.h
        Game/Katamari/Systems/KatamariCameraController.cpp
        Game/Katamari/Systems/KatamariCameraController.h
        Game/Katamari/Systems/KatamariStaticObstacleRenderSystem.cpp
        Game/Katamari/Systems/KatamariStaticObstacleRenderSystem.h
        Game/Katamari/Systems/KatamariSphereWorldResolveSystem.cpp
        Game/Katamari/Systems/KatamariSphereWorldResolveSystem.h
        Game/Katamari/Data/PickupArchetype.h
        Game/Katamari/Data/KatamariGameConfig.h
        Game/Katamari/Data/KatamariWorldContext.h
        Game/Katamari/Data/KatamariPickupCatalog.cpp
        Game/Katamari/Data/KatamariPickupCatalog.h
        Game/Katamari/UI/KatamariHud.cpp
        Game/Katamari/UI/KatamariHud.h
        Game/Katamari/Data/KatamariPickupCatalog.cpp

        Game/LightingTest/LightingTestGame.cpp
        Game/LightingTest/LightingTestGame.h
)

find_package(directxtk CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
target_include_directories(PingPong PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(SolarSystemGame STATIC
        Game/SolarSystem/SolarSystemGame.cpp
        Game/SolarSystem/SolarSystemScene.cpp
)

target_include_directories(SolarSystemGame PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(SolarSystemGame PRIVATE
        Microsoft::DirectXTK
)

target_link_libraries(PingPong PRIVATE
        Microsoft::DirectXTK
        assimp::assimp
        d3d11
        dxgi
        d3dcompiler
        dxguid
        user32
        gdi32
)
file(COPY ${CMAKE_SOURCE_DIR}/Game/Pong/Assets DESTINATION ${CMAKE_BINARY_DIR}/Game/Pong)
file(COPY ${CMAKE_SOURCE_DIR}/Game/SolarSystem/Assets DESTINATION ${CMAKE_BINARY_DIR}/Game/SolarSystem)
if(EXISTS "${CMAKE_SOURCE_DIR}/Game/Katamari/Assets")
    file(COPY ${CMAKE_SOURCE_DIR}/Game/Katamari/Assets DESTINATION ${CMAKE_BINARY_DIR}/Game/Katamari)
endif()
file(COPY
        ${CMAKE_SOURCE_DIR}/Core/Shaders
        DESTINATION ${CMAKE_BINARY_DIR}/Core
)

add_custom_command(
        TARGET PingPong POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/Core/Shaders"
        "${CMAKE_BINARY_DIR}/Core/Shaders"
        COMMENT "Sync Core/Shaders (runtime compile uses build tree copy)"
)

if(EXISTS "${CMAKE_SOURCE_DIR}/Game/Katamari/Assets")
    add_custom_command(
            TARGET PingPong POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/Game/Katamari/Assets"
            "${CMAKE_BINARY_DIR}/Game/Katamari/Assets"
            COMMENT "Sync Katamari assets"
    )
endif()

set(CORE_SHADER_SOURCES
        Core/Shaders/Basic3D.hlsl
        Core/Shaders/BasicPS.hlsl
        Core/Shaders/BasicVS.hlsl
        Core/Shaders/ForwardPhong3D.hlsl
        Core/Shaders/ShadowSamplingCSM.hlsli
        Core/Shaders/DeferredGeometry3D.hlsl
        Core/Shaders/DeferredLighting3D.hlsl
        Core/Shaders/DeferredComposite3D.hlsl
        Core/Shaders/GBufferDebugComposite3D.hlsl
        Core/Shaders/GpuParticlesUpdateCS.hlsl
        Core/Shaders/GpuParticlesBitonicSortCS.hlsl
        Core/Shaders/GpuParticlesDraw3D.hlsl
        Core/Shaders/ShadowDepth3D.hlsl
        Core/Shaders/Shape2D.hlsl
)

target_sources(PingPong PRIVATE ${CORE_SHADER_SOURCES})
set_source_files_properties(${CORE_SHADER_SOURCES} PROPERTIES HEADER_FILE_ONLY TRUE)
source_group("Core/Shaders" FILES ${CORE_SHADER_SOURCES})
